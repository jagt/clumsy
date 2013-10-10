#include <stdlib.h>
#include <Windows.h>
#include "iup.h"
#include "common.h"

short calcChance(short chance) {
    // notice that here we made a copy of chance, so even though it's volatile it is still ok
    return (chance == 1000) || ((rand() & 0x3FF) < chance);
}

static short resolutionSet = 0;

void startTimePeriod() {
    if (!resolutionSet) {
        // begin only fails when period out of range
        timeBeginPeriod(TIMER_RESOLUTION);
        resolutionSet = 1;
    }
}

void endTimePeriod() {
    if (resolutionSet) {
        timeEndPeriod(TIMER_RESOLUTION);
        resolutionSet = 0;
    }
}


// shared callbacks
int uiSyncChance(Ihandle *ih) {
    char valueBuf[8];
    float value = IupGetFloat(ih, "VALUE"), newValue = value;
    short *chancePtr = (short*)IupGetAttribute(ih, SYNCED_VALUE);
    if (newValue > 100.0f) {
       newValue = 100.0f;
    } else if (newValue < 0) {
       newValue = 0.0f;
    }
    if (newValue != value) { // equality compare is fine since newValue is a copy of value
        sprintf(valueBuf, "%.1f", newValue);
        IupStoreAttribute(ih, "VALUE", valueBuf);
        // put caret at last to enable editting while normalizing
        IupStoreAttribute(ih, "CARET", "10");
    }
    // and sync chance value
    InterlockedExchange16(chancePtr, (short)(newValue * 10));
    return IUP_DEFAULT;
}

int uiSyncToggle(Ihandle *ih, int state) {
    short *togglePtr = (short*)IupGetAttribute(ih, SYNCED_VALUE);
    InterlockedExchange16(togglePtr, (short)state);
    return IUP_DEFAULT;
}

int uiSyncInteger(Ihandle *ih) {
    short *integerPointer = (short*)IupGetAttribute(ih, SYNCED_VALUE);
    const int maxValue = IupGetInt(ih, INTEGER_MAX);
    const int minValue = IupGetInt(ih, INTEGER_MIN);
    // normalize input into [min, max]
    int value = IupGetInt(ih, "VALUE"), newValue = value;
    char valueBuf[8];
    if (newValue > maxValue) {
        newValue = maxValue;
    } else if (newValue < minValue) {
        newValue = minValue;
    }
    // test for 0 as for empty input
    if (newValue != value && value != 0) {
        sprintf(valueBuf, "%d", newValue);
        IupStoreAttribute(ih, "VALUE", valueBuf);
        // put caret at last to enable editting while normalizing
        IupStoreAttribute(ih, "CARET", "10");
    }
    // sync back
    InterlockedExchange16(integerPointer, (short)newValue);
    return IUP_DEFAULT;
}

// indicator icon, generated from scripts/im2carr.py
const unsigned char icon8x8[8*8] = {
    0, 0, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 1, 1, 1, 1, 0, 0,
};
