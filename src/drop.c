// dropping packet module
#include <stdlib.h>
#include <Windows.h>
#include "iup.h"
#include "common.h"
#define NAME "drop"

static Ihandle *inboundCheckbox, *outboundCheckbox, *chanceInput;

static volatile short dropEnabled = 0,
    dropInbound = 1, dropOutbound = 1,
    chance = 1000; // [0-10000]


static Ihandle* dropSetupUI() {
    Ihandle *dropControlsBox = IupHbox(
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
    IupSetAttribute(inboundCheckbox, SYNCED_VALUE, (char*)&dropInbound);
    IupSetCallback(outboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(outboundCheckbox, SYNCED_VALUE, (char*)&dropOutbound);

    // enable by default to avoid confusing
    IupSetAttribute(inboundCheckbox, "VALUE", "ON");
    IupSetAttribute(outboundCheckbox, "VALUE", "ON");

    if (parameterized) {
        setFromParameter(inboundCheckbox, "VALUE", NAME"-inbound");
        setFromParameter(outboundCheckbox, "VALUE", NAME"-outbound");
        setFromParameter(chanceInput, "VALUE", NAME"-chance");
    }

    return dropControlsBox;
}

static void dropStartUp() {
    LOG("drop enabled");
}

static void dropCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
    LOG("drop disabled");
}

static short dropProcess(PacketNode *head, PacketNode* tail) {
    int dropped = 0;
    while (head->next != tail) {
        PacketNode *pac = head->next;
        // chance in range of [0, 10000]
        if (checkDirection(pac->addr.Direction, dropInbound, dropOutbound)
            && calcChance(chance)) {
            LOG("dropped with chance %.1f%%, direction %s",
                chance/100.0, BOUND_TEXT(pac->addr.Direction));
            freeNode(popNode(pac));
            ++dropped;
        } else {
            head = head->next;
        }
    }

    return dropped > 0;
}


Module dropModule = {
    "Drop",
    NAME,
    (short*)&dropEnabled,
    dropSetupUI,
    dropStartUp,
    dropCloseDown,
    dropProcess,
    // runtime fields
    0, 0, NULL
};
