// packet length filter module
#include <stdlib.h>
#include <Windows.h>
#include "iup.h"
#include "common.h"
#define NAME "length"

static Ihandle *inboundCheckbox, *outboundCheckbox, *enabledCheckbox;
static Ihandle *minLengthInput, *maxLengthInput;

static volatile short lengthEnabled = 0,
    lengthInbound = 1, lengthOutbound = 1,
    minLength = 64, maxLength = 1500; // [0-65535] bytes


static Ihandle* lengthSetupUI() {
    Ihandle *lengthControlsBox = IupHbox(
        inboundCheckbox = IupToggle("Inbound", NULL),
        outboundCheckbox = IupToggle("Outbound", NULL),
        IupLabel("Min:"),
        minLengthInput = IupText(NULL),
        IupLabel("Max:"),
        maxLengthInput = IupText(NULL),
        IupLabel("bytes"),
        NULL
    );

    // Setup min length input
    IupSetAttribute(minLengthInput, "VISIBLECOLUMNS", "6");
    IupSetAttribute(minLengthInput, "VALUE", "64");
    IupSetCallback(minLengthInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(minLengthInput, SYNCED_VALUE, (char*)&minLength);
    IupSetAttribute(minLengthInput, INTEGER_MIN, "0");
    IupSetAttribute(minLengthInput, INTEGER_MAX, "65535");

    // Setup max length input  
    IupSetAttribute(maxLengthInput, "VISIBLECOLUMNS", "6");
    IupSetAttribute(maxLengthInput, "VALUE", "1500");
    IupSetCallback(maxLengthInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(maxLengthInput, SYNCED_VALUE, (char*)&maxLength);
    IupSetAttribute(maxLengthInput, INTEGER_MIN, "0");
    IupSetAttribute(maxLengthInput, INTEGER_MAX, "65535");

    // Setup direction checkboxes
    IupSetCallback(inboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(inboundCheckbox, SYNCED_VALUE, (char*)&lengthInbound);
    IupSetCallback(outboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(outboundCheckbox, SYNCED_VALUE, (char*)&lengthOutbound);

    // Enable by default to avoid confusion
    IupSetAttribute(inboundCheckbox, "VALUE", "ON");
    IupSetAttribute(outboundCheckbox, "VALUE", "ON");

    if (parameterized) {
        setFromParameter(inboundCheckbox, "VALUE", NAME"-inbound");
        setFromParameter(outboundCheckbox, "VALUE", NAME"-outbound");
        setFromParameter(minLengthInput, "VALUE", NAME"-min");
        setFromParameter(maxLengthInput, "VALUE", NAME"-max");
    }

    return lengthControlsBox;
}

static void lengthStartup() {
    LOG("length filter enabled: min=%d, max=%d", minLength, maxLength);
}

static void lengthCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
    LOG("length filter disabled");
}

static short lengthProcess(PacketNode *head, PacketNode* tail) {
    int processed = 0;
    while (head->next != tail) {
        PacketNode *pac = head->next;
        
        if (checkDirection(pac->addr.Outbound, lengthInbound, lengthOutbound)) {
            UINT packetLength = pac->packetLen;
            
            // Check if packet length is outside the specified range
            if (packetLength < minLength || packetLength > maxLength) {
                LOG("filtered packet with length %u bytes (range: %d-%d), direction %s",
                    packetLength, minLength, maxLength, 
                    pac->addr.Outbound ? "OUTBOUND" : "INBOUND");
                freeNode(popNode(pac));
                ++processed;
            } else {
                head = head->next;
            }
        } else {
            head = head->next;
        }
    }

    return processed > 0;
}

Module lengthModule = {
    "Length Filter",
    NAME,
    (short*)&lengthEnabled,
    lengthSetupUI,
    lengthStartup,
    lengthCloseDown,
    lengthProcess,
    // runtime fields
    0, 0, NULL
};