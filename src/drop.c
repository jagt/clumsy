// dropping packet module
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include "common.h"
#include "iup.h"

static Ihandle *dropControlsBox, *inboundCheckbox, *outboundCheckbox, *chanceInput;

static volatile short dropEnabled = 0;
static volatile short dropInbound = 0, dropOutbound = 0;
static volatile short chance = 50;

static int uiNormalizeChanceValue(Ihandle *ih) {
    char valueBuf[8];
    int value = IupGetInt(ih, "VALUE");
    if (value > 100) {
        value = 100;
    } else if (value < 0) {
        value = 0;
    }
    sprintf(valueBuf, "%d", value);
    IupStoreAttribute(ih, "VALUE", valueBuf);
    // put caret at last to enable editting while normalizing
    IupStoreAttribute(ih, "CARET", "10");
    // and sync chance value
    InterlockedExchange16(&chance, value);
    return IUP_DEFAULT;
}

static int uiSyncDropInbound(Ihandle *ih) {
    int ret = IupGetInt(ih, "VALUE"); // IupGetInt handles YES/NO, ON/OFF
    LOG("dropInbound:%d", ret);
    InterlockedExchange16(&dropInbound, ret);
    return IUP_DEFAULT;
}

static int uiSyncDropOutbound(Ihandle *ih) {
    int ret = IupGetInt(ih, "VALUE");
    InterlockedExchange16(&dropOutbound, ret);
    return IUP_DEFAULT;
}

static Ihandle* setupDropUI() {
    dropControlsBox = IupHbox(
        inboundCheckbox = IupToggle("Drop Incoming", NULL),
        outboundCheckbox = IupToggle("Drop Outgoing", NULL),
        IupLabel("Chance(%):"),
        chanceInput = IupText(NULL),
        NULL
    );

    IupSetAttribute(chanceInput, "VISIBLECOLUMNS", "2");
    IupSetAttribute(chanceInput, "VALUE", "50");
    IupSetCallback(chanceInput, "VALUECHANGED_CB", uiNormalizeChanceValue);
    IupSetCallback(inboundCheckbox, "VALUECHANGED_CB", uiSyncDropInbound);
    IupSetCallback(outboundCheckbox, "VALUECHANGED_CB", uiSyncDropOutbound);

    return dropControlsBox;
}

static void dropStartUp() {
    LOG("drop enabled");
    srand(time(NULL));
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
        doDrop = (chance == 100) || ((rand() & 0x3FF) < chance * 10);
        if (doDrop &&
            ((dropInbound && (pac->addr.Direction == DIVERT_DIRECTION_INBOUND)) 
             || (dropOutbound && (pac->addr.Direction == DIVERT_DIRECTION_OUTBOUND))
            )) {
            LOG("droped with chance %d, direction %s",
                chance, pac->addr.Direction == DIVERT_DIRECTION_INBOUND ? "IN" : "OUT");
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
