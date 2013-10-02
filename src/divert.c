#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include "divert.h"
#include "common.h"
#define CLUMPSY_DIVERT_PRIORITY 0
#define MAX_PACKETSIZE 0xFFFF
#define READ_TIME_PER_STEP 3

static HANDLE divertHandle;

static PackageNode headNode = {0}, tailNode = {0};
static PackageNode * const head = &headNode, * const tail = &tailNode;

PackageNode* createNode(char* buf, UINT len, DIVERT_ADDRESS *addr) {
    PackageNode *newNode = (PackageNode*)malloc(sizeof(PackageNode));
    newNode->packet = (char*)malloc(len);
    newNode->packetLen = len;
    memcpy(&(newNode->addr), addr, sizeof(DIVERT_ADDRESS));
    newNode->next = newNode->prev = NULL;
    return newNode;
}

void freeNode(PackageNode *node) {
    assert((node != head) && (node != tail));
    free(node->packet);
    free(node);
}

PackageNode* popNode(PackageNode *node) {
    assert((node != head) && (node != tail));
    node->prev->next = node->next;
    node->next->prev = node->prev;
    return node;
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

    return 1;
}

void divertReadLoop() {
    int ix = 0;
    char packetBuf[MAX_PACKETSIZE];
    DIVERT_ADDRESS addrBuf;
    UINT readLen;
    while (1) {
        if (!DivertRecv(divertHandle, packetBuf, MAX_PACKETSIZE, &addrBuf, &readLen)) {
            puts("Failed to recv a packet.");
            continue;
        }
        if (readLen > MAX_PACKETSIZE) {
            puts("Interal Error: DivertRecv truncated packate,"); // don't know how this can happen
        }

        // create node and put it into the list

    }
}

void divertStop() {
    // FIXME not sure how DivertClose is failing
    assert(DivertClose(divertHandle));
}
