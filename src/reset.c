// Reset injection packet module
#include <stdlib.h>
#include "iup.h"
#include "common.h"
#define NAME "reset"

static Ihandle *inboundCheckbox, *outboundCheckbox, *chanceInput, *countInput;

static volatile short resetEnabled = 0,
    resetInbound = 1, resetOutbound = 1,
    chance = 1; // [0-1000]

static Ihandle* resetSetupUI() {
    Ihandle *dupControlsBox = IupHbox(
        inboundCheckbox = IupToggle("Inbound", NULL),
        outboundCheckbox = IupToggle("Outbound", NULL),
        IupLabel("Chance(%):"),
        chanceInput = IupText(NULL),
        NULL
        );

    IupSetAttribute(chanceInput, "VISIBLECOLUMNS", "4");
    IupSetAttribute(chanceInput, "VALUE", "0.1");
    IupSetCallback(chanceInput, "VALUECHANGED_CB", uiSyncChance);
    IupSetAttribute(chanceInput, SYNCED_VALUE, (char*)&chance);
    IupSetCallback(inboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(inboundCheckbox, SYNCED_VALUE, (char*)&resetInbound);
    IupSetCallback(outboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(outboundCheckbox, SYNCED_VALUE, (char*)&resetOutbound);

    // enable by default to avoid confusing
    IupSetAttribute(inboundCheckbox, "VALUE", "ON");
    IupSetAttribute(outboundCheckbox, "VALUE", "ON");

    if (parameterized) {
        setFromParameter(inboundCheckbox, "VALUE", NAME"-inbound");
        setFromParameter(outboundCheckbox, "VALUE", NAME"-outbound");
        setFromParameter(chanceInput, "VALUE", NAME"-chance");
    }

    return dupControlsBox;
}

static void resetStartup() {
    LOG("reset enabled");
}

static void resetCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
    LOG("reset disabled");
}

static short resetProcess(PacketNode *head, PacketNode *tail) {
    short reset = FALSE;
    PacketNode *pac = head->next;
    while (pac != tail) {
        if (checkDirection(pac->addr.Direction, resetInbound, resetOutbound)
            && calcChance(chance)
            && pac->packetLen > 33)
        {
            LOG("injecting reset w/ chance %.1f%%", chance/10.0);
            pac->packet[33] |= 4;
            WinDivertHelperCalcChecksums(pac->packet, pac->packetLen, 0);

            reset = TRUE;
        }
        
        pac = pac->next;
    }
    return reset;
}

Module resetModule = {
    "Set reset flag",
    NAME,
    (short*)&resetEnabled,
    resetSetupUI,
    resetStartup,
    resetCloseDown,
    resetProcess,
    // runtime fields
    0, 0, NULL
};