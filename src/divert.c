#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include "divert.h"
#include "common.h"
#define CLUMPSY_DIVERT_PRIORITY 0
#define MAX_PACKETSIZE 0xFFFF
#define READ_TIME_PER_STEP 3

static HANDLE divertHandle;

static PacketNode headNode = {0}, tailNode = {0};
static PacketNode * const head = &headNode, * const tail = &tailNode;

static volatile short stopLooping;
static DWORD divertReadLoop(LPVOID arg);
static HANDLE loopThread;

PacketNode* createNode(char* buf, UINT len, DIVERT_ADDRESS *addr) {
    PacketNode *newNode = (PacketNode*)malloc(sizeof(PacketNode));
    newNode->packet = (char*)malloc(len);
    newNode->packetLen = len;
    memcpy(&(newNode->addr), addr, sizeof(DIVERT_ADDRESS));
    newNode->next = newNode->prev = NULL;
    return newNode;
}

void freeNode(PacketNode *node) {
    assert((node != head) && (node != tail));
    free(node->packet);
    free(node);
}

PacketNode* popNode(PacketNode *node) {
    assert((node != head) && (node != tail));
    node->prev->next = node->next;
    node->next->prev = node->prev;
    return node;
}

PacketNode* insertAfter(PacketNode *node, PacketNode *target) {
    node->prev = target;
    node->next = target->next;
    target->next->prev = node;
    target->next = node;
    return node;
}

PacketNode* insertBefore(PacketNode *node, PacketNode *target) {
    node->next = target;
    node->prev = target->prev;
    target->prev->next = node;
    target->prev = node;
}

PacketNode* appendNode(PacketNode *node) {
    return insertBefore(node, tail);
}

int divertStart(const char * filter, char buf[]) {
    int ix;
    divertHandle = DivertOpen(filter, DIVERT_LAYER_NETWORK, CLUMPSY_DIVERT_PRIORITY, 0);
    if (divertHandle == INVALID_HANDLE_VALUE) {
        DWORD lastError = GetLastError();
        if (lastError == ERROR_INVALID_PARAMETER) {
            strcpy(buf, "Failed to start filtering : filter syntax error.");
        } else {
            sprintf(buf, "Failed to start filtering : failed to open device %d", lastError);
        }
        return 0;
    }

    // init package link list
    head->next = tail;
    tail->prev = head;

    // reset module
    for (ix = 0; ix < MODULE_CNT; ++ix) {
        modules[ix]->lastEnabled = 0;
    }

    // kick off the loop
    stopLooping = 0;
    loopThread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)divertReadLoop, NULL, 0, NULL);

    return 1;
}

static DWORD divertReadLoop(LPVOID arg) {
    int ix = 0;
    char packetBuf[MAX_PACKETSIZE];
    DIVERT_ADDRESS addrBuf;
    UINT readLen, sendLen;
    PacketNode *pnode;

    PDIVERT_IPHDR ipheader;
    while (1) {
        /*
        if (!DivertRecv(divertHandle, packetBuf, MAX_PACKETSIZE, &addrBuf, &readLen)) {
            puts("Failed to recv a packet.");
            continue;
        }
        if (readLen > MAX_PACKETSIZE) {
            // don't know how this can happen
            puts("Interal Error: DivertRecv truncated recv packet."); 
        }

        // create node and put it into the list
        pnode = createNode(packetBuf, readLen, &addrBuf);
        appendNode(pnode);

        // FIXME processing with modules and check enable/disable status


        // send packet from tail to head and remove sent ones
        while (head->next != tail) {
            pnode = popNode(tail->prev);
            if (!DivertSend(divertHandle, pnode->packet, pnode->packetLen, &(pnode->addr), &sendLen)) {
                printf("Failed to send a packet. (%d)\n", GetLastError());
            }
            if (sendLen < pnode->packetLen) {
                // don't know how this can happen, or it needs to resent like good old UDP packet
                puts("Internal Error: DivertSend truncated send packet.");
            }
            freeNode(pnode);
        }

        if (stopLooping) {
            break;
        }
        */
        // Read a matching packet.
        if (!DivertRecv(divertHandle, packetBuf, MAX_PACKETSIZE, &addrBuf, &readLen))
        {
            fprintf(stderr, "warning: failed to read packet (%d)\n",
                GetLastError());
            continue;
        }

        DivertHelperParse(packetBuf, readLen, &ipheader, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        printf("%d, %d\n", ipheader->SrcAddr, ipheader->DstAddr);
       
        // Re-inject the matching packet.
        if (!DivertSend(divertHandle, packetBuf, readLen, &addrBuf, &sendLen))
        {
            fprintf(stderr, "warning: failed to reinject packet (%d)\n",
                GetLastError());
        }

        if (stopLooping) {
            break;
        }
    }

    // FIXME clean ups

    return 0;
}

void divertStop() {
    InterlockedIncrement16(&stopLooping);
    WaitForSingleObject(loopThread, INFINITE);
    // FIXME not sure how DivertClose is failing
    assert(DivertClose(divertHandle));
}
