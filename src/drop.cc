// dropping packet module
// implement functionalities using plain functions because fuck classes
#include <cstdio>
#include "common.h"
#include "iup.h"

static Ihandle *dropControlsBox, *inboundCheckbox, *outboundCheckbox, *chanceInput;

static bool dropInbound = true, dropOutbound = true;
static int chance = 50;

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
    return IUP_DEFAULT;
}

static Ihandle* setupDropUI() {
    puts("what the fuck man!");
    IupVbox(IupVbox(NULL));
    dropControlsBox = IupLabel(
        //inboundCheckbox = IupToggle("Drop Incoming", NULL),
        //outboundCheckbox = IupToggle("Drop Outgoing", NULL),
        //IupLabel("Chance(%):"),
        //chanceInput = IupText(NULL)
        "What the fuck man!"
    );

    /*
    IupSetAttribute(chanceInput, "VISIBLECOLUMNS", "2");
    IupSetAttribute(chanceInput, "VALUE", "50");
    IupSetCallback(chanceInput, "VALUECHANGED_CB", uiNormalizeChanceValue);
    */

    return dropControlsBox;
}

Module dropModule = {
    "Drop",
    setupDropUI
};
