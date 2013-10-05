// throttling packets
#include "iup.h"
#include "common.h"
#define MIN_PACKETS "2"
#define MAX_PACKETS "100"

static Ihandle *inboundCheckbox, *outboundCheckbox, *chanceInput, *numInput;

static volatile short throttleEnabled = 0,
    throttleInbound = 1, throttleOutbound = 1,
    chance = 100, // [0-1000]
    throttleNumber = 4; // throttle how many packets in one step

static PacketNode throttleHeadNode = {0}, throttleTailNode = {0};
static PacketNode *bufHead = &throttleHeadNode, *bufTail = &throttleTailNode;

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
}


static void throttleCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
}

static void throttleProcess(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
}

Module throttleModule = {
    "Throttle",
    (short*)&throttleEnabled,
    throttleSetupUI,
    throttleStartUp,
    throttleCloseDown,
    throttleProcess
};