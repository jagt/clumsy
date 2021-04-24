#include <stdlib.h>
#include <memory.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include "windivert.h"
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
PWINDIVERT_IPHDR dbg_ip_header;
PWINDIVERT_IPV6HDR dbg_ipv6_header;
PWINDIVERT_TCPHDR dbg_tcp_header;
PWINDIVERT_UDPHDR dbg_udp_header;
PWINDIVERT_ICMPHDR dbg_icmp_header;
PWINDIVERT_ICMPV6HDR dbg_icmpv6_header;
UINT payload_len;
void dumpPacket(char *buf, int len, PWINDIVERT_ADDRESS paddr) {
    char *protocol;
    UINT16 srcPort = 0, dstPort = 0;

    WinDivertHelperParsePacket(buf, len, &dbg_ip_header, &dbg_ipv6_header, NULL,
        &dbg_icmp_header, &dbg_icmpv6_header, &dbg_tcp_header, &dbg_udp_header,
        NULL, &payload_len, NULL, NULL);
    // need to cast byte order on port numbers
    if (dbg_tcp_header != NULL) {
        protocol = "TCP ";
        srcPort = ntohs(dbg_tcp_header->SrcPort);
        dstPort = ntohs(dbg_tcp_header->DstPort);
    } else if (dbg_udp_header != NULL) {
        protocol = "UDP ";
        srcPort = ntohs(dbg_udp_header->SrcPort);
        dstPort = ntohs(dbg_udp_header->DstPort);
    } else if (dbg_icmp_header || dbg_icmpv6_header) {
        protocol = "ICMP";
    } else {
        protocol = "???";
    }

    if (dbg_ip_header != NULL) {
        UINT8 *src_addr = (UINT8*)&dbg_ip_header->SrcAddr;
        UINT8 *dst_addr = (UINT8*)&dbg_ip_header->DstAddr;
        LOG("%s.%s: %u.%u.%u.%u:%d->%u.%u.%u.%u:%d",
            protocol,
            paddr->Outbound ? "OUT " : "IN  ",
            src_addr[0], src_addr[1], src_addr[2], src_addr[3], srcPort,
            dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3], dstPort);
    } else if (dbg_ipv6_header != NULL) {
        UINT16 *src_addr6 = (UINT16*)&dbg_ipv6_header->SrcAddr;
        UINT16 *dst_addr6 = (UINT16*)&dbg_ipv6_header->DstAddr;
        LOG("%s.%s: %x:%x:%x:%x:%x:%x:%x:%x:%d->%x:%x:%x:%x:%x:%x:%x:%x:%d",
            protocol,
            paddr->Outbound ? "OUT " : "IN  ",
            src_addr6[0], src_addr6[1], src_addr6[2], src_addr6[3],
            src_addr6[4], src_addr6[5], src_addr6[6], src_addr6[7], srcPort,
            dst_addr6[0], dst_addr6[1], dst_addr6[2], dst_addr6[3],
            dst_addr6[4], dst_addr6[5], dst_addr6[6], dst_addr6[7], dstPort);
    }
}
#else
#define dumpPacket(x, y, z)
#endif

int divertStart(const char *filter, char buf[]) {
    int ix;

    divertHandle = WinDivertOpen(filter, WINDIVERT_LAYER_NETWORK, DIVERT_PRIORITY, 0);
    if (divertHandle == INVALID_HANDLE_VALUE) {
        DWORD lastError = GetLastError();
        if (lastError == ERROR_INVALID_PARAMETER) {
            strcpy(buf, "Failed to start filtering : filter syntax error.");
        } else {
            sprintf(buf, "Failed to start filtering : failed to open device (code:%lu).\n"
                "Make sure you run clumsy as Administrator.", lastError);
        }
        return FALSE;
    }
    LOG("Divert opened handle.");

    WinDivertSetParam(divertHandle, WINDIVERT_PARAM_QUEUE_LENGTH, QUEUE_LEN);
    WinDivertSetParam(divertHandle, WINDIVERT_PARAM_QUEUE_TIME, QUEUE_TIME);
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
        sprintf(buf, "Failed to create mutex (%lu)", GetLastError());
        return FALSE;
    }

    loopThread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)divertReadLoop, NULL, 0, NULL);
    if (loopThread == NULL) {
        sprintf(buf, "Failed to create recv loop thread (%lu)", GetLastError());
        return FALSE;
    }
    clockThread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)divertClockLoop, NULL, 0, NULL);
    if (clockThread == NULL) {
        sprintf(buf, "Failed to create clock loop thread (%lu)", GetLastError());
        return FALSE;
    }

    LOG("Threads created");

    return TRUE;
}

static int sendAllListPackets() {
    // send packet from tail to head and remove sent ones
    int sendCount = 0;
    UINT sendLen;
    PacketNode *pnode;
#ifdef _DEBUG
    // check the list is good
    // might go into dead loop but it's better for debugging
    PacketNode *p = head;
    do {
        p = p->next;
    } while (p->next);
    assert(p == tail);
#endif

    while (!isListEmpty()) {
        pnode = popNode(tail->prev);
        sendLen = 0;
        assert(pnode != head);
        // FIXME inbound injection on any kind of packet is failing with a very high percentage
        //       need to contact windivert auther and wait for next release
        if (!WinDivertSend(divertHandle, pnode->packet, pnode->packetLen, &sendLen, &(pnode->addr))) {
            PWINDIVERT_ICMPHDR icmp_header;
            PWINDIVERT_ICMPV6HDR icmpv6_header;
            PWINDIVERT_IPHDR ip_header;
            PWINDIVERT_IPV6HDR ipv6_header;
            LOG("Failed to send a packet. (%lu)", GetLastError());
            dumpPacket(pnode->packet, pnode->packetLen, &(pnode->addr));
            // as noted in windivert help, reinject inbound icmp packets some times would fail
            // workaround this by resend them as outbound
            // TODO not sure is this even working as can't find a way to test
            //      need to document about this
            WinDivertHelperParsePacket(pnode->packet, pnode->packetLen, &ip_header, &ipv6_header, NULL,
                &icmp_header, &icmpv6_header, NULL, NULL, NULL, NULL, NULL, NULL);
            if ((icmp_header || icmpv6_header) && !pnode->addr.Outbound) {
                BOOL resent;
                pnode->addr.Outbound = TRUE;
                if (ip_header) {
                    UINT32 tmp = ip_header->SrcAddr;
                    ip_header->SrcAddr = ip_header->DstAddr;
                    ip_header->DstAddr = tmp;
                } else if (ipv6_header) {
                    UINT32 tmpArr[4];
                    memcpy(tmpArr, ipv6_header->SrcAddr, sizeof(tmpArr));
                    memcpy(ipv6_header->SrcAddr, ipv6_header->DstAddr, sizeof(tmpArr));
                    memcpy(ipv6_header->DstAddr, tmpArr, sizeof(tmpArr));
                }
                resent = WinDivertSend(divertHandle, pnode->packet, pnode->packetLen, &sendLen, &(pnode->addr));
                LOG("Resend failed inbound ICMP packets as outbound: %s", resent ? "SUCCESS" : "FAIL");
                InterlockedExchange16(&sendState, SEND_STATUS_SEND);
            } else {
                InterlockedExchange16(&sendState, SEND_STATUS_FAIL);
            }
        } else {
            if (sendLen < pnode->packetLen) {
                // TODO don't know how this can happen, or it needs to be resent like good old UDP packet
                LOG("Internal Error: DivertSend truncated send packet.");
                InterlockedExchange16(&sendState, SEND_STATUS_FAIL);
            } else {
                InterlockedExchange16(&sendState, SEND_STATUS_SEND);
            }
        }


        freeNode(pnode);
        ++sendCount;
    }
    assert(isListEmpty()); // all packets should be sent by now

    return sendCount;
}

// step function to let module process and consume all packets on the list
static void divertConsumeStep() {
#ifdef _DEBUG
    DWORD startTick = GetTickCount(), dt;
#endif
    int ix, cnt;
    // use lastEnabled to keep track of module starting up and closing down
    for (ix = 0; ix < MODULE_CNT; ++ix) {
        Module *module = modules[ix];
        if (*(module->enabledFlag)) {
            if (!module->lastEnabled) {
                module->startUp();
                module->lastEnabled = 1;
            }
            if (module->process(head, tail)) {
                InterlockedIncrement16(&(module->processTriggered));
            }
        } else {
            if (module->lastEnabled) {
                module->closeDown(head, tail);
                module->lastEnabled = 0;
            }
        }
    }
    cnt = sendAllListPackets();
#ifdef _DEBUG
    dt =  GetTickCount() - startTick;
    if (dt > CLOCK_WAITMS / 2) {
        LOG("Costy consume step: %lu ms, sent %d packets", GetTickCount() - startTick, cnt);
    }
#endif
}

// periodically try to consume packets to keep the network responsive and not blocked by recv
static DWORD divertClockLoop(LPVOID arg) {
    DWORD startTick, stepTick, waitResult;
    int ix;

    UNREFERENCED_PARAMETER(arg);

    for(;;) {
        // use acquire as wait for yielding thread
        startTick = GetTickCount();
        waitResult = WaitForSingleObject(mutex, CLOCK_WAITMS);
        switch(waitResult) {
            case WAIT_OBJECT_0:
                /***************** enter critical region ************************/
                divertConsumeStep();
                /***************** leave critical region ************************/
                if (!ReleaseMutex(mutex)) {
                    InterlockedIncrement16(&stopLooping);
                    LOG("Fatal: Failed to release mutex (%lu)", GetLastError());
                    ABORT();
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
                LOG("Acquired abandoned mutex");
                InterlockedIncrement16(&stopLooping);
                break;
            case WAIT_FAILED:
                LOG("Acquire failed (%lu)", GetLastError());
                InterlockedIncrement16(&stopLooping);
                break;
        }

        // need to get the lock here
        if (stopLooping) {
            int lastSendCount = 0;
            BOOL closed;

            waitResult = WaitForSingleObject(mutex, INFINITE);
            switch (waitResult)
            {
            case WAIT_ABANDONED:
            case WAIT_FAILED:
                LOG("Acquire failed/abandoned mutex (%lu), will still try closing and return", GetLastError());
            case WAIT_OBJECT_0:
                /***************** enter critical region ************************/
                LOG("Read stopLooping, stopping...");
                // clean up by closing all modules
                for (ix = 0; ix < MODULE_CNT; ++ix) {
                    Module *module = modules[ix];
                    if (*(module->enabledFlag)) {
                        module->closeDown(head, tail);
                    } 
                }
                LOG("Send all packets upon closing");
                lastSendCount = sendAllListPackets();
                LOG("Lastly sent %d packets. Closing...", lastSendCount);

                // terminate recv loop by closing handler. handle related error in recv loop to quit
                closed = WinDivertClose(divertHandle);
                assert(closed);

                // release to let read loop exit properly
                /***************** leave critical region ************************/
                if (!ReleaseMutex(mutex)) {
                    LOG("Fatal: Failed to release mutex (%lu)", GetLastError());
                    ABORT();
                }
                return 0;
                break;
            }
        }
    }
}

static DWORD divertReadLoop(LPVOID arg) {
    char packetBuf[MAX_PACKETSIZE];
    WINDIVERT_ADDRESS addrBuf;
    UINT readLen;
    PacketNode *pnode;
    DWORD waitResult;

    UNREFERENCED_PARAMETER(arg);

    for(;;) {
        // each step must fully consume the list
        assert(isListEmpty()); // FIXME has failed this assert before. don't know why
        if (!WinDivertRecv(divertHandle, packetBuf, MAX_PACKETSIZE, &readLen, &addrBuf)) {
            DWORD lastError = GetLastError();
            if (lastError == ERROR_INVALID_HANDLE || lastError == ERROR_OPERATION_ABORTED) {
                // treat closing handle as quit
                LOG("Handle died or operation aborted. Exit loop.");
                return 0;
            }
            LOG("Failed to recv a packet. (%lu)", GetLastError());
            continue;
        }
        if (readLen > MAX_PACKETSIZE) {
            // don't know how this can happen
            LOG("Internal Error: DivertRecv truncated recv packet."); 
        }

        //dumpPacket(packetBuf, readLen, &addrBuf);  

        waitResult = WaitForSingleObject(mutex, INFINITE);
        switch(waitResult) {
            case WAIT_OBJECT_0:
                /***************** enter critical region ************************/
                if (stopLooping) {
                    LOG("Lost last recved packet but user stopped. Stop read loop.");
                    /***************** leave critical region ************************/
                    if (!ReleaseMutex(mutex)) {
                        LOG("Fatal: Failed to release mutex on stopping (%lu). Will stop anyway.", GetLastError());
                    }
                    return 0;
                }
                // create node and put it into the list
                pnode = createNode(packetBuf, readLen, &addrBuf);
                appendNode(pnode);
                divertConsumeStep();
                /***************** leave critical region ************************/
                if (!ReleaseMutex(mutex)) {
                    LOG("Fatal: Failed to release mutex (%lu)", GetLastError());
                    ABORT();
                }
                break;
            case WAIT_TIMEOUT:
                LOG("Acquire timeout, dropping one read packet");
                continue;
                break;
            case WAIT_ABANDONED:
                LOG("Acquire abandoned.");
                return 0;
            case WAIT_FAILED:
                LOG("Acquire failed.");
                return 0;
        }
    }
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
