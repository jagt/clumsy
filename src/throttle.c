// throttling packets
#include "iup.h"
#include "common.h"
#define TIME_MIN "0"
#define TIME_MAX "1000"
#define TIME_DEFAULT 30
// threshold for how many packet to throttle at most
#define KEEP_AT_MOST 1000

static Ihandle *inboundCheckbox, *outboundCheckbox, *chanceInput, *frameInput;

static volatile short throttleEnabled = 0,
    throttleInbound = 1, throttleOutbound = 1,
    chance = 100, // [0-1000]
    // time frame in ms, when a throttle start the packets within the time 
    // will be queued and sent altogether when time is over
    throttleFrame = TIME_DEFAULT; 

static PacketNode throttleHeadNode = {0}, throttleTailNode = {0};
static PacketNode *bufHead = &throttleHeadNode, *bufTail = &throttleTailNode;
static int bufSize = 0;
static DWORD throttleStartTick = 0;

static INLINE_FUNCTION short isBufEmpty() {
    short ret = bufHead->next == bufTail;
    if (ret) assert(bufSize == 0);
    return ret;
}

static Ihandle *throttleSetupUI() {
    Ihandle *throttleControlsBox = IupHbox(
        IupLabel("Timeframe(ms):"),
        frameInput = IupText(NULL),
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
    IupSetAttribute(frameInput, "VISIBLECOLUMNS", "3");
    IupSetAttribute(frameInput, "VALUE", STR(TIME_DEFAULT));
    IupSetCallback(frameInput, "VALUECHANGED_CB", (Icallback)uiSyncInteger);
    IupSetAttribute(frameInput, SYNCED_VALUE, (char*)&throttleFrame);
    IupSetAttribute(frameInput, INTEGER_MAX, TIME_MAX);
    IupSetAttribute(frameInput, INTEGER_MIN, TIME_MIN);

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
    throttleStartTick = 0;
    startTimePeriod();
}

static void clearBufPackets(PacketNode *tail) {
    PacketNode *oldLast = tail->prev;
    LOG("Throttled end, send all %d packets. Buffer at max: %s", bufSize, bufSize == KEEP_AT_MOST ? "YES" : "NO");
    while (!isBufEmpty()) {
        insertAfter(popNode(bufTail->prev), oldLast);
        --bufSize;
    }
    throttleStartTick = 0;
}

static void throttleCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(tail);
    UNREFERENCED_PARAMETER(head);
    clearBufPackets(tail);
    endTimePeriod();
}

static short throttleProcess(PacketNode *head, PacketNode *tail) {
    short throttled = FALSE;
    UNREFERENCED_PARAMETER(head);
    if (!throttleStartTick) {
        if (!isListEmpty() && calcChance(chance)) {
            LOG("Start new throttling w/ chance %.1f, time frame: %d", chance/10.0, throttleFrame);
            throttleStartTick = timeGetTime();
            throttled = TRUE;
            goto THROTTLE_START; // need this goto since maybe we'll start and stop at this single call
        }
    } else {
THROTTLE_START:
        // start a block for declaring local variables
        {
            // already throttling, keep filling up
            PacketNode *pac = tail->prev;
            DWORD currentTick = timeGetTime();
            while (bufSize < KEEP_AT_MOST && pac != head) {
                if (checkDirection(pac->addr.Direction, throttleInbound, throttleOutbound)) {
                    insertAfter(popNode(pac), bufHead);
                    ++bufSize;
                    pac = tail->prev;
                } else {
                    pac = pac->prev;
                }
            }

            // send all when throttled enough, including in current step
            if (bufSize >= KEEP_AT_MOST || (currentTick - throttleStartTick > (unsigned int)throttleFrame)) {
                clearBufPackets(tail);
            }
        }
    }

    return throttled;
}

Module throttleModule = {
    "Throttle",
    (short*)&throttleEnabled,
    throttleSetupUI,
    throttleStartUp,
    throttleCloseDown,
    throttleProcess
};