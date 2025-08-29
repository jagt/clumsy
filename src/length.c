// packet length filter module
#include <stdlib.h>
#include <Windows.h>
#include "iup.h"
#include "common.h"
#define NAME "length"
#define KEEP_AT_MOST 2000
#define LAG_TIME_DEFAULT 50

// Action definitions
typedef enum {
    LENGTH_ACTION_PASS = 0,
    LENGTH_ACTION_DROP = 1,
    LENGTH_ACTION_LAG = 2,
    LENGTH_ACTION_DUPLICATE = 3
} LengthAction;

static Ihandle *inboundCheckbox, *outboundCheckbox, *enabledCheckbox;
static Ihandle *minLengthInput, *maxLengthInput;
static Ihandle *actionList; // Dropdown for action selection
static Ihandle *lagTimeInput; // Input for lag time when using lag action
static Ihandle *duplicateCountInput; // Input for duplicate count when using duplicate action

// Buffer for lagged packets
static PacketNode lengthLagHeadNode = {0}, lengthLagTailNode = {0};
static PacketNode *lagBufHead = &lengthLagHeadNode, *lagBufTail = &lengthLagTailNode;
static int lagBufSize = 0;

static volatile short lengthEnabled = 0,
    lengthInbound = 1, lengthOutbound = 1,
    minLength = 64, maxLength = 1500, // [0-65535] bytes
    selectedAction = LENGTH_ACTION_DROP, // Default action
    lagTime = LAG_TIME_DEFAULT, // Default lag time in ms
    duplicateCount = 2; // Default duplicate count

static INLINE_FUNCTION short isLagBufEmpty() {
    short ret = lagBufHead->next == lagBufTail;
    if (ret) assert(lagBufSize == 0);
    return ret;
}

static Ihandle* lengthSetupUI() {
    // Create a more compact layout by grouping related controls
    Ihandle *directionBox = IupHbox(
        inboundCheckbox = IupToggle("Inbound", NULL),
        outboundCheckbox = IupToggle("Outbound", NULL),
        NULL
    );
    
    Ihandle *lengthBox = IupHbox(
        IupLabel("Min:"),
        minLengthInput = IupText(NULL),
        IupLabel("Max:"),
        maxLengthInput = IupText(NULL),
        IupLabel("bytes"),
        NULL
    );
    
    Ihandle *actionBox = IupHbox(
        IupLabel("Action:"),
        actionList = IupList(NULL),
        NULL
    );
    
    Ihandle *configBox = IupHbox(
        IupLabel("Lag(ms):"),
        lagTimeInput = IupText(NULL),
        IupLabel("Dup Count:"),
        duplicateCountInput = IupText(NULL),
        NULL
    );
    
    Ihandle *lengthControlsBox = IupVbox(
        directionBox,
        lengthBox,
        actionBox,
        configBox,
        NULL
    );

    // Setup min length input
    IupSetAttribute(minLengthInput, "VISIBLECOLUMNS", "4");
    IupSetAttribute(minLengthInput, "VALUE", "64");
    IupSetCallback(minLengthInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(minLengthInput, SYNCED_VALUE, (char*)&minLength);
    IupSetAttribute(minLengthInput, INTEGER_MIN, "0");
    IupSetAttribute(minLengthInput, INTEGER_MAX, "65535");

    // Setup max length input  
    IupSetAttribute(maxLengthInput, "VISIBLECOLUMNS", "4");
    IupSetAttribute(maxLengthInput, "VALUE", "1500");
    IupSetCallback(maxLengthInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(maxLengthInput, SYNCED_VALUE, (char*)&maxLength);
    IupSetAttribute(maxLengthInput, INTEGER_MIN, "0");
    IupSetAttribute(maxLengthInput, INTEGER_MAX, "65535");

    // Setup action list
    IupSetAttribute(actionList, "DROPDOWN", "YES");
    IupSetAttribute(actionList, "VISIBLECOLUMNS", "10");
    IupSetAttribute(actionList, "1", "Pass Through");
    IupSetAttribute(actionList, "2", "Drop");
    IupSetAttribute(actionList, "3", "Lag");
    IupSetAttribute(actionList, "4", "Duplicate");
    IupSetAttribute(actionList, "VALUE", "2"); // Default to Drop
    IupSetCallback(actionList, "ACTION", (Icallback)uiSyncInteger);
    IupSetAttribute(actionList, SYNCED_VALUE, (char*)&selectedAction);

    // Setup lag time input
    IupSetAttribute(lagTimeInput, "VISIBLECOLUMNS", "3");
    IupSetAttribute(lagTimeInput, "VALUE", STR(LAG_TIME_DEFAULT));
    IupSetCallback(lagTimeInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(lagTimeInput, SYNCED_VALUE, (char*)&lagTime);
    IupSetAttribute(lagTimeInput, INTEGER_MAX, "15000");
    IupSetAttribute(lagTimeInput, INTEGER_MIN, "0");

    // Setup duplicate count input
    IupSetAttribute(duplicateCountInput, "VISIBLECOLUMNS", "2");
    IupSetAttribute(duplicateCountInput, "VALUE", "2");
    IupSetCallback(duplicateCountInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(duplicateCountInput, SYNCED_VALUE, (char*)&duplicateCount);
    IupSetAttribute(duplicateCountInput, INTEGER_MAX, "50");
    IupSetAttribute(duplicateCountInput, INTEGER_MIN, "2");

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
        setFromParameter(actionList, "VALUE", NAME"-action");
        setFromParameter(lagTimeInput, "VALUE", NAME"-lagtime");
        setFromParameter(duplicateCountInput, "VALUE", NAME"-dupcount");
    }

    return lengthControlsBox;
}

static void lengthStartup() {
    // Initialize lag buffer
    if (lagBufHead->next == NULL && lagBufTail->next == NULL) {
        lagBufHead->next = lagBufTail;
        lagBufTail->prev = lagBufHead;
        lagBufSize = 0;
    } else {
        assert(isLagBufEmpty());
    }
    
    LOG("length filter enabled: min=%d, max=%d, action=%d, lagTime=%d, duplicateCount=%d", 
        minLength, maxLength, selectedAction, lagTime, duplicateCount);
    startTimePeriod();
}

static void flushLagBuffer(PacketNode *tail) {
    PacketNode *oldLast = tail->prev;
    // flush all buffered packets
    LOG("Flushing lag buffer, %d packets", lagBufSize);
    while(!isLagBufEmpty()) {
        insertAfter(popNode(lagBufTail->prev), oldLast);
        --lagBufSize;
    }
}

static void lengthCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    // Flush all buffered packets when closing down
    flushLagBuffer(tail);
    LOG("length filter disabled");
    endTimePeriod();
}

static short lengthProcess(PacketNode *head, PacketNode* tail) {
    int processed = 0;
    DWORD currentTime = timeGetTime();
    
    // First, try to send any lagged packets that are due
    if (selectedAction == LENGTH_ACTION_LAG) {
        while (!isLagBufEmpty()) {
            PacketNode *pac = lagBufTail->prev;
            if (currentTime > pac->timestamp + lagTime) {
                insertAfter(popNode(lagBufTail->prev), head); // sending queue is already empty by now
                --lagBufSize;
                // Log the packet action when it's sent with lag
                logPacketAction(pac, "LENGTH_LAG", "length");
                LOG("Send lagged packets from length filter.");
                ++processed;
            } else {
                break;
            }
        }
    }
    
    while (head->next != tail) {
        PacketNode *pac = head->next;
        
        if (checkDirection(pac->addr.Outbound, lengthInbound, lengthOutbound)) {
            UINT packetLength = pac->packetLen;
            
            // Check if packet length is within the specified range
            if (packetLength >= minLength && packetLength <= maxLength) {
                // Packet matches the length filter, apply selected action
                switch (selectedAction) {
                    case LENGTH_ACTION_DROP:
                        LOG("dropped packet with length %u bytes (range: %d-%d), direction %s",
                            packetLength, minLength, maxLength, 
                            pac->addr.Outbound ? "OUTBOUND" : "INBOUND");
                        logPacketAction(pac, "LENGTH_DROP", "length");
                        freeNode(popNode(pac));
                        ++processed;
                        break;
                        
                    case LENGTH_ACTION_LAG:
                        // Add packet to lag buffer with timestamp
                        if (lagBufSize < KEEP_AT_MOST) {
                            PacketNode *moved = popNode(pac);
                            moved->timestamp = timeGetTime();
                            insertAfter(moved, lagBufHead);
                            ++lagBufSize;
                            LOG("Added packet to lag buffer, size: %d", lagBufSize);
                            logPacketAction(moved, "LENGTH_LAG", "length");
                        } else {
                            // Buffer full, just pass through
                            head = head->next;
                        }
                        ++processed;
                        break;
                        
                    case LENGTH_ACTION_DUPLICATE:
                        {
                            short copies = duplicateCount - 1;
                            LOG("duplicating packet with length %u bytes, cloned additionally %d packets", 
                                packetLength, copies);
                            while (copies-- > 0) {
                                PacketNode *copy = createNode(pac->packet, pac->packetLen, &(pac->addr));
                                insertBefore(copy, pac); // must insertBefore or next packet is still pac
                                // Log the packet action for each duplicate
                                logPacketAction(copy, "LENGTH_DUPLICATE", "length");
                            }
                            // Log the original packet action
                            logPacketAction(pac, "LENGTH_DUPLICATE", "length");
                            head = head->next;
                            ++processed;
                        }
                        break;
                        
                    case LENGTH_ACTION_PASS:
                    default:
                        // Pass through without modification
                        head = head->next;
                        break;
                }
            } else {
                // Packet doesn't match the length filter, pass through
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