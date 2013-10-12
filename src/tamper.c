// tampering packet module
#include "iup.h"
#include "common.h"

static Ihandle *inboundCheckbox, *outboundCheckbox, *chanceInput, *checksumCheckbox;

static volatile short tamperEnabled = 0,
    tamperInbound = 1,
    tamperOutbound = 1,
    chance = 100, // [0 - 1000]
    doChecksum = 1; // whether redo after tampering checksum

static Ihandle* tamperSetupUI() {
    Ihandle *dupControlsBox = IupHbox(
        checksumCheckbox = IupToggle("Redo Checksum", NULL),
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
    IupSetAttribute(inboundCheckbox, SYNCED_VALUE, (char*)&tamperInbound);
    IupSetCallback(outboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(outboundCheckbox, SYNCED_VALUE, (char*)&tamperOutbound);
    // sync doChecksum
    IupSetCallback(checksumCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(checksumCheckbox, SYNCED_VALUE, (char*)&doChecksum);

    // enable by default to avoid confusing
    IupSetAttribute(inboundCheckbox, "VALUE", "ON");
    IupSetAttribute(outboundCheckbox, "VALUE", "ON");
    IupSetAttribute(checksumCheckbox, "VALUE", "ON");

    return dupControlsBox;
}

static void tamperStartup() {
}

static void tamperCloseDown(PacketNode *head, PacketNode *tail) {
}

static short tamperProcess(PacketNode *head, PacketNode *tail) {
    return FALSE;
}

Module tamperModule = {
    "Tamper",
    (short*)&tamperEnabled,
    tamperSetupUI,
    tamperStartup,
    tamperCloseDown,
    tamperProcess
};
