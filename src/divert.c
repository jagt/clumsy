#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include "divert.h"
#include "common.h"
#define CLUMPSY_DIVERT_PRIORITY 0
#define MAX_PACKETSIZE 0xFFFF
#define READ_TIME_PER_STEP 3

static HANDLE divertHandle;
static volatile short stopLooping;
static DWORD divertReadLoop(LPVOID arg);
static HANDLE loopThread;


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
