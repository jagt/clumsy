// throttling packets
#include "iup.h"
#include "common.h"
#define MIN_PACKETS "1"
#define MAX_PACKETS "100"

static Ihandle *inboundCheckbox, *outboundCheckbox, *chanceInput, *numInput;

static volatile short throttleEnabled = 0,
    throttleInbound = 1, throttleOutbound = 1,
    chance = 100, // [0-1000]
    throttleNumber = 4; // throttle how many packets in one step

static PacketNode throttleHeadNode = {0}, throttleTailNode = {0};
static PacketNode *bufHead = &throttleHeadNode, *bufTail = &throttleTailNode;
static int bufSize = 0;

static short isBufEmpty() {
    short ret = bufHead->next == bufTail;
    if (ret) assert(bufSize == 0);
    return ret;
}

static Ihandle *throttleSetupUI() {
    Ihandle *throttleControlsBox = IupHbox(
        IupLabel("Count:"),
        numInput = IupText(NULL),
        inboundCheckbox = IupToggle("Inbound", NULL),
        outboundCheckbox = IupToggle("Outbound", NULL),
        IupLabel("Chance(%):"),
        chanceInput = IupText(NULL),
        NULL
        );

    IupSetAttribute(chanceInput, "VISIBLECOLUMNS", "4");
    IupSetAttribute(chanceInput, "VALUE", "10.0");
    IupSetCallback(chanceInput, "VALUECHANGED_CB", uiSyncChance);
    IupSetAttribute(chanceInput, SYNCED_VALUE, (char*)&chance);
    IupSetCallback(inboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(inboundCheckbox, SYNCED_VALUE, (char*)&throttleInbound);
    IupSetCallback(outboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(outboundCheckbox, SYNCED_VALUE, (char*)&throttleOutbound);
    // sync throttle packet number
    IupSetAttribute(numInput, "VISIBLECOLUMNS", "4");
    IupSetAttribute(numInput, "VALUE", "4");
    IupSetCallback(numInput, "VALUECHANGED_CB", (Icallback)uiSyncInteger);
    IupSetAttribute(numInput, SYNCED_VALUE, (char*)&throttleNumber);
    IupSetAttribute(numInput, INTEGER_MAX, MAX_PACKETS);
    IupSetAttribute(numInput, INTEGER_MIN, MIN_PACKETS);

    // enable by default to avoid confusing
    IupSetAttribute(inboundCheckbox, "VALUE", "ON");
    IupSetAttribute(outboundCheckbox, "VALUE", "ON");

    return throttleControlsBox;
}

static void throttleStartUp() {
    if (bufHead->next == NULL && bufTail->next == NULL) {
        bufHead->next = bufTail;
        bufTail->prev = bufHead;
        bufSize = 0;
    } else {
        assert(isBufEmpty());
    }
}

static void clearBufPackets(PacketNode *tail) {
    PacketNode *oldLast = tail->prev;
    LOG("Throttled enough, send all.");
    while (!isBufEmpty()) {
        insertAfter(popNode(bufTail->prev), oldLast);
        --bufSize;
    }
}

static void throttleCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(tail);
    UNREFERENCED_PARAMETER(head);
    clearBufPackets(tail);
}

static void throttleProcess(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    if (isBufEmpty()) {
        // only calculate chance when having a packet
        if (!isListEmpty() && calcChance(chance)) {
            LOG("Start new throttling w/ chance %.1f, aim at %d packets", chance/10.0, throttleNumber);
            goto THROTTLE_START;
        }
    } else {
THROTTLE_START:
        // start a block for declaring limit
        {
            // make a copy of volatile number, just to be safe
            short limit = throttleNumber;
            // already throttling, keep filling up
            while (bufSize < limit && !isListEmpty()) {
                insertAfter(popNode(tail->prev), bufHead);
                ++bufSize;
            }

            // send all when throttled enough, including in current step
            if (bufSize >= limit) {
                clearBufPackets(tail);
            }
        }
    }
}

Module throttleModule = {
    "Throttle",
    (short*)&throttleEnabled,
    throttleSetupUI,
    throttleStartUp,
    throttleCloseDown,
    throttleProcess
};