// duplicate packet module
#include <stdlib.h>
#include "iup.h"
#include "common.h"
#define COPIES_MIN "2"
#define COPIES_MAX "50"
#define COPIES_COUNT 2

static Ihandle *inboundCheckbox, *outboundCheckbox, *chanceInput, *countInput;

static volatile short dupEnabled = 0,
    dupInbound = 1, dupOutbound = 1,
    chance = 100, // [0-1000]
    count = COPIES_COUNT; // how many copies to duplicate

static Ihandle* dupSetupUI() {
    Ihandle *dupControlsBox = IupHbox(
        IupLabel("Count:"),
        countInput = IupText(NULL),
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
    IupSetAttribute(inboundCheckbox, SYNCED_VALUE, (char*)&dupInbound);
    IupSetCallback(outboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(outboundCheckbox, SYNCED_VALUE, (char*)&dupOutbound);
    // sync count
    IupSetAttribute(countInput, "VISIBLECOLUMNS", "3");
    IupSetAttribute(countInput, "VALUE", STR(COPIES_COUNT));
    IupSetCallback(countInput, "VALUECHANGED_CB", (Icallback)uiSyncInteger);
    IupSetAttribute(countInput, SYNCED_VALUE, (char*)&count);
    IupSetAttribute(countInput, INTEGER_MAX, COPIES_MAX);
    IupSetAttribute(countInput, INTEGER_MIN, COPIES_MIN);

    // enable by default to avoid confusing
    IupSetAttribute(inboundCheckbox, "VALUE", "ON");
    IupSetAttribute(outboundCheckbox, "VALUE", "ON");

    return dupControlsBox;
}

static void dupStartup() {
    LOG("dup enabled");
}

static void dupCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
    LOG("dup disabled");
}

static short dupProcess(PacketNode *head, PacketNode *tail) {
    return 1;
}

Module dupModule = {
    "Duplicate",
    (short*)&dupEnabled,
    dupSetupUI,
    dupStartup,
    dupCloseDown,
    dupProcess
};