// dropping packet module
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include "iup.h"
#include "common.h"

static Ihandle *inboundCheckbox, *outboundCheckbox, *chanceInput;

static volatile short dropEnabled = 0;
static volatile short dropInbound = 0, dropOutbound = 0;
static volatile short chance = 100; // [0-1000]


static Ihandle* setupDropUI() {
    Ihandle *dropControlsBox = IupHbox(
        inboundCheckbox = IupToggle("Drop Incoming", NULL),
        outboundCheckbox = IupToggle("Drop Outgoing", NULL),
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

    return dropControlsBox;
}

static void dropStartUp() {
    LOG("drop enabled");
    srand((unsigned int)time(NULL));
}

static void dropCloseDown() {
    LOG("drop disabled");
}

static void dropProcess(PacketNode *head, PacketNode* tail) {
    short doDrop;
    while (head->next != tail) {
        PacketNode *pac = head->next;
        // due to the threading issue the chance here may change between the first
        // and the second read. I think I'm aware of this but it's fine here i suppose.
        // chance in range of [0, 1000]
        doDrop = (chance == 1000) || ((rand() & 0x3FF) < chance);
        if (doDrop &&
            ((dropInbound && (pac->addr.Direction == DIVERT_DIRECTION_INBOUND)) 
             || (dropOutbound && (pac->addr.Direction == DIVERT_DIRECTION_OUTBOUND))
            )) {
            LOG("droped with chance %.1f% , direction %s",
                chance/10.0, pac->addr.Direction == DIVERT_DIRECTION_INBOUND ? "IN" : "OUT");
            freeNode(popNode(pac));
        } else {
            head = head->next;
        }
    }
}


Module dropModule = {
    "Drop",
    (short*)&dropEnabled,
    setupDropUI,
    dropStartUp,
    dropCloseDown,
    dropProcess
};
