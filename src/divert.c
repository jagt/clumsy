#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <winsock2.h>
#include "divert.h"
#include "common.h"
#define DIVERT_PRIORITY 0
#define MAX_PACKETSIZE 0xFFFF
#define READ_TIME_PER_STEP 3
// FIXME does this need to be larger then the time to process the list?
#define CLOCK_WAITMS 40
#define QUEUE_LEN 2 << 10
#define QUEUE_TIME 2 << 9 

static HANDLE divertHandle;
static volatile short stopLooping;
static HANDLE loopThread, clockThread, mutex;

static DWORD divertReadLoop(LPVOID arg);
static DWORD divertClockLoop(LPVOID arg);

// not to put these in common.h since modules shouldn't see these
extern PacketNode * const head;
extern PacketNode * const tail;

#ifdef _DEBUG
PDIVERT_IPHDR ip_header;
PDIVERT_IPV6HDR ipv6_header;
PDIVERT_TCPHDR tcp_header;
PDIVERT_UDPHDR udp_header;
UINT payload_len;
void dumpPacket(char *buf, int len, PDIVERT_ADDRESS paddr) {
    UINT16 srcPort = 0, dstPort = 0;

    DivertHelperParsePacket(buf, len, &ip_header, &ipv6_header,
        NULL, NULL, &tcp_header, &udp_header,
        NULL, &payload_len);
    // need to cast byte order on port numbers
    if (tcp_header != NULL) {
        srcPort = ntohs(tcp_header->SrcPort);
        dstPort = ntohs(tcp_header->DstPort);
    } else if (udp_header != NULL) {
        srcPort = ntohs(udp_header->SrcPort);
        dstPort = ntohs(udp_header->DstPort);
    }

    if (ip_header != NULL) {
        UINT8 *src_addr = (UINT8*)&ip_header->SrcAddr;
        UINT8 *dst_addr = (UINT8*)&ip_header->DstAddr;
        LOG("%s: %u.%u.%u.%u:%d->%u.%u.%u.%u:%d",
            paddr->Direction == DIVERT_DIRECTION_OUTBOUND ? "OUT " : "IN  ",
            src_addr[0], src_addr[1], src_addr[2], src_addr[3], srcPort,
            dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3], dstPort);
    } else if (ipv6_header != NULL) {
        UINT16 *src_addr6 = (UINT16*)&ipv6_header->SrcAddr;
        UINT16 *dst_addr6 = (UINT16*)&ipv6_header->DstAddr;
        LOG("%s: %x:%x:%x:%x:%x:%x:%x:%x:%d->%x:%x:%x:%x:%x:%x:%x:%x:%d",
            paddr->Direction == DIVERT_DIRECTION_OUTBOUND ? "OUT " : "IN  ",
            src_addr6[0], src_addr6[1], src_addr6[2], src_addr6[3],
            src_addr6[4], src_addr6[5], src_addr6[6], src_addr6[7], srcPort,
            dst_addr6[0], dst_addr6[1], dst_addr6[2], dst_addr6[3],
            dst_addr6[4], dst_addr6[5], dst_addr6[6], dst_addr6[7], dstPort);
    }
}
#else
#define dumpPacket(x, y, z)
#endif

int divertStart(const char * filter, char buf[]) {
    int ix;
    char fixedFilter[MSG_BUFSIZE];

    // injecting icmp packets is quite likely to fail, as described in windivert documentation
    // so as a workaround we disable icmp in the filter, and filter out icmp packets
    // FIXME "ip", "inbound", "outbound" still includes icmp so this is basically broken
    if (strstr(filter, "icmp")) { // includes "icmpv6"
        strcpy(buf, "'icmp'/'icmpv6' is not allowed in the filter. clumsy ignores all icmp packets (refer to faqs for further info).");
        return FALSE;
    }
    sprintf(fixedFilter, "%s and not icmp and not icmpv6", filter);

    LOG("Fixed Filter: %s", fixedFilter);
    divertHandle = DivertOpen(fixedFilter, DIVERT_LAYER_NETWORK, DIVERT_PRIORITY, 0);
    if (divertHandle == INVALID_HANDLE_VALUE) {
        DWORD lastError = GetLastError();
        if (lastError == ERROR_INVALID_PARAMETER) {
            strcpy(buf, "Failed to start filtering : filter syntax error.");
        } else {
            sprintf(buf, "Failed to start filtering : failed to open device %d", lastError);
        }
        return FALSE;
    }
    LOG("Divert opened handle: %d", divertHandle);

    DivertSetParam(divertHandle, DIVERT_PARAM_QUEUE_LEN, QUEUE_LEN);
    DivertSetParam(divertHandle, DIVERT_PARAM_QUEUE_TIME, QUEUE_TIME);
    LOG("WinDivert internal queue Len: %d, queue time: %d", QUEUE_LEN, QUEUE_TIME);

    // init package link list
    initPacketNodeList();

    // reset module
    for (ix = 0; ix < MODULE_CNT; ++ix) {
        modules[ix]->lastEnabled = 0;
    }

    // kick off the loop
    LOG("Creating threads and mutex...");
    stopLooping = FALSE;
    mutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL) {
        sprintf(buf, "Failed to create mutex (%d)", GetLastError());
        return FALSE;
    }

    loopThread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)divertReadLoop, NULL, 0, NULL);
    if (loopThread == NULL) {
        sprintf(buf, "Failed to create recv loop thread (%d)", GetLastError());
        return FALSE;
    }
    clockThread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)divertClockLoop, NULL, 0, NULL);
    if (clockThread == NULL) {
        sprintf(buf, "Failed to create clock loop thread (%d)", GetLastError());
        return FALSE;
    }

    LOG("Threads created: %d", loopThread);

    return TRUE;
}

// step function to let module process and consume all packets on the list
static void divertConsumeStep() {
#ifdef _DEBUG
    DWORD startTick = GetTickCount(), dt;
    int cnt = 0;
#endif
    PacketNode *pnode;
    UINT sendLen;
    int ix;
    // use lastEnabled to keep track of module starting up and closing down
    for (ix = 0; ix < MODULE_CNT; ++ix) {
        Module *module = modules[ix];
        if (*(module->enabledFlag)) {
            if (!module->lastEnabled) {
                module->startUp();
                module->lastEnabled = 1;
            }
            module->process(head, tail);
        } else {
            if (module->lastEnabled) {
                module->closeDown(head, tail);
                module->lastEnabled = 0;
            }
        }
    }

    // send packet from tail to head and remove sent ones
    while (!isListEmpty()) {
        pnode = popNode(tail->prev);
        assert(pnode != head);
        if (!DivertSend(divertHandle, pnode->packet, pnode->packetLen, &(pnode->addr), &sendLen)) {
            LOG("Failed to send a packet. (%d)", GetLastError());
        }
        if (sendLen < pnode->packetLen) {
            // don't know how this can happen, or it needs to resent like good old UDP packet
            LOG("Internal Error: DivertSend truncated send packet.");
        }
        freeNode(pnode);
#ifdef _DEBUG
        ++cnt;
#endif
    }
#ifdef _DEBUG
    dt =  GetTickCount() - startTick;
    if (dt > CLOCK_WAITMS / 2) {
        LOG("Costy consume step: %d ms, sent packets: %d", GetTickCount() - startTick, cnt);
    }
#endif
}

// periodically try to consume packets to keep the network responsive and not blocked by recv
static DWORD divertClockLoop(LPVOID arg) {
    DWORD startTick, stepTick, waitResult;
    int ix;

    UNREFERENCED_PARAMETER(arg);

    for(;;) {
        // use aquire as wait for yielding thread
        startTick = GetTickCount();
        waitResult = WaitForSingleObject(mutex, CLOCK_WAITMS);
        switch(waitResult) {
            case WAIT_OBJECT_0:
                divertConsumeStep();
                if (!ReleaseMutex(mutex)) {
                    InterlockedIncrement16(&stopLooping);
                    LOG("Failed to release mutex (%d)", GetLastError());
                }
                // if didn't spent enough time, we sleep on it
                stepTick = GetTickCount() - startTick;
                if (stepTick < CLOCK_WAITMS) {
                    Sleep(CLOCK_WAITMS - stepTick);
                }
                break;
            case WAIT_TIMEOUT:
                // read loop is processing, so we can skip this run
                LOG("!!! Skipping one run");
                Sleep(CLOCK_WAITMS);
                break;
            case WAIT_ABANDONED:
                LOG("Aquired abandoned mutex");
                InterlockedIncrement16(&stopLooping);
                break;
            case WAIT_FAILED:
                LOG("Aquire failed (%d)", GetLastError());
                InterlockedIncrement16(&stopLooping);
                break;
        }

        if (stopLooping) {
            LOG("Read stopLooping, stopping...");
            // clean up by closing all modules
            for (ix = 0; ix < MODULE_CNT; ++ix) {
                Module *module = modules[ix];
                if (*(module->enabledFlag)) {
                    module->closeDown(head, tail);
                } 
            }
            LOG("Cleaning up modules.");

            // terminate recv loop by closing handler. handle related error in recv loop to quit
            assert(DivertClose(divertHandle));
            return 0;
        }
    }
}

static DWORD divertReadLoop(LPVOID arg) {
    char packetBuf[MAX_PACKETSIZE];
    DIVERT_ADDRESS addrBuf;
    UINT readLen;
    PacketNode *pnode;
    DWORD waitResult;

    UNREFERENCED_PARAMETER(arg);

    for(;;) {
        // each step must fully consume the list
        assert(isListEmpty());
        if (!DivertRecv(divertHandle, packetBuf, MAX_PACKETSIZE, &addrBuf, &readLen)) {
            DWORD lastError = GetLastError();
            if (lastError == ERROR_INVALID_HANDLE || lastError == ERROR_OPERATION_ABORTED) {
                // treat closing handle as quit
                return 0;
            }
            LOG("Failed to recv a packet. (%d)", GetLastError());
            continue;
        }
        if (readLen > MAX_PACKETSIZE) {
            // don't know how this can happen
            LOG("Interal Error: DivertRecv truncated recv packet."); 
        }

        //dumpPacket(packetBuf, readLen, &addrBuf);  

        waitResult = WaitForSingleObject(mutex, INFINITE);
        switch(waitResult) {
            case WAIT_OBJECT_0:
                // create node and put it into the list
                pnode = createNode(packetBuf, readLen, &addrBuf);
                appendNode(pnode);
                divertConsumeStep();
                if (!ReleaseMutex(mutex)) {
                    LOG("Failed to release mutex (%d)", GetLastError());
                    return 0;
                }
                break;
            case WAIT_TIMEOUT:
                LOG("Aquire timeout, dropping one read packet");
                continue;
                break;
            case WAIT_ABANDONED:
                LOG("Aquire abandoned.");
                return 0;
            case WAIT_FAILED:
                LOG("Aquire failed.");
                return 0;
        }
        if (stopLooping) {
            // still allow exit gracefully
            LOG("Stop read loop.");
            break;
        }
    }

    return 0;
}

void divertStop() {
    HANDLE threads[2];
    threads[0] = loopThread;
    threads[1] = clockThread;

    LOG("Stopping...");
    InterlockedIncrement16(&stopLooping);
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    LOG("Successfully waited threads and stopped.");
}
