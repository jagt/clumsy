// dropping packet module
#include <stdio.h>
#include <Windows.h>
#include "common.h"
#include "iup.h"

static Ihandle *dropControlsBox, *inboundCheckbox, *outboundCheckbox, *chanceInput;

static volatile short dropEnabled = 0;
static volatile short dropInbound = 1, dropOutbound = 1;
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
    int ret = IS_YES(IupGetAttribute(ih, "VALUE"));
    InterlockedExchange16(&dropInbound, ret);
    return IUP_DEFAULT;
}

static int uiSyncDropOutbound(Ihandle *ih) {
    int ret = IS_YES(IupGetAttribute(ih, "VALUE"));
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

Module dropModule = {
    "Drop",
    (short*)&dropEnabled,
    setupDropUI
};
