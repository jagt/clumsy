#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include "divert.h"
#include "common.h"
#define CLUMPSY_DIVERT_PRIORITY 0
#define MAX_PACKETSIZE 0xFFFF
#define READ_TIME_PER_STEP 3

PackageNode *head;
static PackageNode headNode;
static HANDLE divertHandle;
static char packetBuf[MAX_PACKETSIZE];
static DIVERT_ADDRESS addrBuf;

PackageNode* createNode(char* buf, UINT len, DIVERT_ADDRESS *addr) {
    PackageNode *newNode = (PackageNode*)malloc(sizeof(PackageNode));
    newNode->packet = (char*)malloc(len);
    newNode->packetLen = len;
    memcpy(&(newNode->addr), addr, sizeof(DIVERT_ADDRESS));
    newNode->next = newNode->prev = NULL;
    return newNode;
}

int divertStart(const char * filter, char buf[]) {
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
    memset(&headNode, 0, sizeof(PackageNode));
    head = &headNode;

    return 1;
}

void divertRead() {
    int ix = 0;
    UINT readLen;
    while (ix < READ_TIME_PER_STEP) {
        if (!DivertRecv(divertHandle, packetBuf, MAX_PACKETSIZE, &addrBuf, &readLen)) {
            showStatus("Failed to recv a packet.");
            continue;
        }
        if (readLen > MAX_PACKETSIZE) {
            showStatus("Interal Error: DivertRecv truncated packate,"); // don't know how this can happen
        }



        ++ix;
    }
}

void divertStop() {
    // FIXME not sure how DivertClose is failing
    assert(DivertClose(divertHandle));
}
