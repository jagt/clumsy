#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <winsock2.h>
#include "divert.h"
#include "common.h"
#define CLUMPSY_DIVERT_PRIORITY 0
#define MAX_PACKETSIZE 0xFFFF
#define READ_TIME_PER_STEP 3

static HANDLE divertHandle;
static volatile short stopLooping;
static DWORD divertReadLoop(LPVOID arg);
static HANDLE loopThread;

#ifdef _DEBUG
PDIVERT_IPHDR ip_header;
PDIVERT_IPV6HDR ipv6_header;
PDIVERT_TCPHDR tcp_header;
PDIVERT_UDPHDR udp_header;
UINT payload_len;
//char pacBuf[MSG_BUFSIZE];
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
    if (strstr(filter, "icmp")) { // includes "icmpv6"
        strcpy(buf, "'icmp'/'icmpv6' is not allowed in the filter. clumpsy ignores all icmp packets (refer to faqs for further info).");
        return FALSE;
    }
    sprintf(fixedFilter, "%s and not icmp and not icmpv6", filter);

    LOG("Fixed Filter: %s", fixedFilter);
    divertHandle = DivertOpen(fixedFilter, DIVERT_LAYER_NETWORK, CLUMPSY_DIVERT_PRIORITY, 0);
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

    // init package link list
    initPacketNodeList();

    // reset module
    for (ix = 0; ix < MODULE_CNT; ++ix) {
        modules[ix]->lastEnabled = 0;
    }

    // kick off the loop
    LOG("Kicking off thread...");
    stopLooping = 0;
    loopThread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)divertReadLoop, NULL, 0, NULL);
    LOG("Thread created: %d", loopThread);

    return TRUE;
}

static DWORD divertReadLoop(LPVOID arg) {
    int ix;
    char packetBuf[MAX_PACKETSIZE];
    DIVERT_ADDRESS addrBuf;
    UINT readLen, sendLen;
    PacketNode *pnode;

    PDIVERT_IPHDR ipheader;
    LOG("Loop thread started...");
    while (1) {
        if (!DivertRecv(divertHandle, packetBuf, MAX_PACKETSIZE, &addrBuf, &readLen)) {
            LOG("Failed to recv a packet.");
            continue;
        }
        if (readLen > MAX_PACKETSIZE) {
            // don't know how this can happen
            LOG("Interal Error: DivertRecv truncated recv packet."); 
        }

        dumpPacket(packetBuf, readLen, &addrBuf);  
        // create node and put it into the list
        pnode = createNode(packetBuf, readLen, &addrBuf);
        appendNode(pnode);

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
                    module->closeDown();
                    module->lastEnabled = 0;
                }
            }
        }

        // send packet from tail to head and remove sent ones
        while (!isListEmpty()) {
            pnode = popNode(tail->prev);
            if (!DivertSend(divertHandle, pnode->packet, pnode->packetLen, &(pnode->addr), &sendLen)) {
                LOG("Failed to send a packet. (%d)", GetLastError());
            }
            if (sendLen < pnode->packetLen) {
                // don't know how this can happen, or it needs to resent like good old UDP packet
                LOG("Internal Error: DivertSend truncated send packet.");
            }
            freeNode(pnode);
        }

        if (stopLooping) {
            LOG("Read stopLooping, stopping...");
            break;
        }
    }

    // FIXME clean ups

    return 0;
}

void divertStop() {
    InterlockedIncrement16(&stopLooping);
    WaitForSingleObject(loopThread, INFINITE);
    LOG("Successfully waited thread %d exit.", loopThread);
    // FIXME not sure how DivertClose is failing
    assert(DivertClose(divertHandle));
}
