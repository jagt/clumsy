#include <stdlib.h>
#include <Windows.h>
#include "iup.h"
#include "common.h"

short calcChance(short chance) {
    // notice that here we made a copy of chance, so even though it's volatile it is still ok
    return (chance == 10000) || ((rand() % 10000) < chance);
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
        // put caret at end to enable editing while normalizing
        IupStoreAttribute(ih, "CARET", "10");
    }
    // and sync chance value
    InterlockedExchange16(chancePtr, (short)(newValue * 100));
    return IUP_DEFAULT;
}

int uiSyncToggle(Ihandle *ih, int state) {
    short *togglePtr = (short*)IupGetAttribute(ih, SYNCED_VALUE);
    InterlockedExchange16(togglePtr, I2S(state));
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
        // put caret at end to enable editing while normalizing
        IupStoreAttribute(ih, "CARET", "10");
    }
    // sync back
    InterlockedExchange16(integerPointer, (short)newValue);
    return IUP_DEFAULT;
}

// naive fixed number of (short) * 0.01
int uiSyncFixed(Ihandle *ih) {
    short *fixedPointer = (short*)IupGetAttribute(ih, SYNCED_VALUE);
    const float maxFixedValue = IupGetFloat(ih, FIXED_MAX);
    const float minFixedValue = IupGetFloat(ih, FIXED_MIN);
    float value = IupGetFloat(ih, "VALUE");
    float newValue = value;
    short fixValue;
    char valueBuf[8];
    if (newValue > maxFixedValue) {
        newValue = maxFixedValue;
    } else if (newValue < minFixedValue) {
        newValue = minFixedValue;
    }

    if (newValue != value && value != 0) {
        sprintf(valueBuf, "%.2f", newValue);
        IupStoreAttribute(ih, "VALUE", valueBuf);
        // put caret at end to enable editing while normalizing
        IupStoreAttribute(ih, "CARET", "10");
    }
    // sync back
    fixValue = (short)(newValue / FIXED_EPSILON);
    InterlockedExchange16(fixedPointer, fixValue);
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

typedef int (*IstateCallback)(Ihandle *ih, int state);
// parameterized setter
void setFromParameter(Ihandle *ih, const char *field, const char *key) {
    char* val = IupGetGlobal(key);
    Icallback cb;
    IstateCallback scb;
    // FIXME there should be a way to trigger handler
    // manually trigger the callback, as iup won't call it
    // Notice that currently only works on IupToggle, IupText
    // and Iup lacks a way to get widget's type, so can't do much about this
    if (val) {
        IupSetAttribute(ih, field, val);
        cb = IupGetCallback(ih, "VALUECHANGED_CB");
        if (cb) {
            LOG("triggered VALUECHANGED_CB on key: %s", key);
            cb(ih);
            return;
        }
        // cb's argument type IS NOT ONLY Ihandle, receives a extra "state" int
        scb = (IstateCallback)IupGetCallback(ih, "ACTION");
        if (scb) {
            LOG("triggered ACTION on key: %s", key);
            // IupGetInt handles yes/no on/off upper lower case things.
            scb(ih, IupGetInt(ih, "VALUE"));
            return;
        }
    }
}

// parse arguments and set globals
// only checks for argument style, no extra validation is done
BOOL parseArgs(int argc, char* argv[]) {
    int ix = 0;
    char *key, *value;
    // begin parsing "--key value" args. 
    // notice that quoted arg with spaces in between is already handled by shell
    if (argc == 1) return 0;
    for (;;) {
        if (++ix >= argc) break;
        key = argv[ix];
        if (key[0] != '-' || key[1] != '-' || key[2] == '\0') {
            return 0;
        }
        key = &(key[2]); // skip "--"
        if (++ix >= argc) {
            return 0;
        }
        value = argv[ix];
        IupStoreGlobal(key, value);
        LOG("option: %s : %s", key, value);
    }

    return 1;
}