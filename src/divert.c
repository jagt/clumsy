#include <stdio.h>
#include <assert.h>
#include "divert.h"
#include "common.h"
#define CLUMPSY_DIVERT_PRIORITY 0

static HANDLE divertHandle;

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
    return 1;
}

void divertStop() {
    // FIXME not sure how DivertClose is failing
    assert(DivertClose(divertHandle));
}
