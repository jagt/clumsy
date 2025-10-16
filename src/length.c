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
    LENGTH_ACTION_NOTHING = 0,
    LENGTH_ACTION_DROP = 1,
    LENGTH_ACTION_LAG = 2,
    LENGTH_ACTION_DUPLICATE = 3,
    LENGTH_ACTION_RESET = 4,
    LENGTH_ACTION_OOO = 5
} LengthAction;

static Ihandle *inboundCheckbox, *outboundCheckbox, *enabledCheckbox;
static Ihandle *minLengthInput, *maxLengthInput;
static Ihandle *actionList; // Dropdown for action selection (matched packets)
static Ihandle *actionListUnmatched; // Dropdown for action selection (non-matched packets)
static Ihandle *lagTimeInput; // Input for lag time when using lag action
static Ihandle *lagTimeInputUnmatched; // Input for lag time for non-matched packets
static Ihandle *duplicateCountInput; // Input for duplicate count when using duplicate action
static Ihandle *duplicateCountInputUnmatched; // Input for duplicate count for non-matched packets

// Buffer for lagged packets
static PacketNode lengthLagHeadNode = {0}, lengthLagTailNode = {0};
static PacketNode *lagBufHead = &lengthLagHeadNode, *lagBufTail = &lengthLagTailNode;
static int lagBufSize = 0;

static volatile short lengthEnabled = 0,
    lengthInbound = 1, lengthOutbound = 1,
    minLength = 64, maxLength = 1500, // [0-65535] bytes
    selectedAction = LENGTH_ACTION_DROP, // Default action for matched packets
    selectedActionUnmatched = LENGTH_ACTION_NOTHING, // Default action for non-matched packets
    lagTime = LAG_TIME_DEFAULT, // Default lag time in ms for matched packets
    lagTimeUnmatched = LAG_TIME_DEFAULT, // Default lag time for non-matched packets
    duplicateCount = 2, // Default duplicate count for matched packets
    duplicateCountUnmatched = 2; // Default duplicate count for non-matched packets

static INLINE_FUNCTION short isLagBufEmpty() {
    short ret = lagBufHead->next == lagBufTail;
    if (ret) assert(lagBufSize == 0);
    return ret;
}

static Ihandle* lengthSetupUI() {
    // Create a single-line layout like throttle, all controls in one Hbox
    Ihandle *lengthControlsBox = IupHbox(
        inboundCheckbox = IupToggle("In", NULL),
        outboundCheckbox = IupToggle("Out", NULL),
        IupLabel("Min:"),
        minLengthInput = IupText(NULL),
        IupLabel("Max:"),
        maxLengthInput = IupText(NULL),
        IupLabel("bytes"),
        IupLabel("Action:"),
        actionList = IupList(NULL),
        IupLabel("Lag:"),
        lagTimeInput = IupText(NULL),
        IupLabel("ms"),
        IupLabel("Dup:"),
        duplicateCountInput = IupText(NULL),
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

    // Setup action list (for matched packets)
    IupSetAttribute(actionList, "DROPDOWN", "YES");
    IupSetAttribute(actionList, "READONLY", "YES");
    IupSetAttribute(actionList, "VISIBLECOLUMNS", "15");
    IupSetAttribute(actionList, "1", "Nothing (Pass)");
    IupSetAttribute(actionList, "2", "Drop");
    IupSetAttribute(actionList, "3", "Lag");
    IupSetAttribute(actionList, "4", "Duplicate");
    IupSetAttribute(actionList, "5", "Reset");
    IupSetAttribute(actionList, "6", "Out of Order");
    IupSetAttribute(actionList, "VALUE", "2"); // Default to Drop
    IupSetCallback(actionList, "VALUECHANGED_CB", (Icallback)uiSyncInteger);
    IupSetAttribute(actionList, SYNCED_VALUE, (char*)&selectedAction);
    IupSetAttribute(actionList, INTEGER_MAX, "6");
    IupSetAttribute(actionList, INTEGER_MIN, "1");

    // Prepare unmatched action list but don't show it in UI (keep for logic only)
    IupSetAttribute(actionListUnmatched, "DROPDOWN", "YES");
    IupSetAttribute(actionListUnmatched, "VISIBLECOLUMNS", "1");
    IupSetAttribute(actionListUnmatched, "1", "Nothing (Pass)");
    IupSetAttribute(actionListUnmatched, "VALUE", "1"); // Default to Nothing (Pass)
    IupSetCallback(actionListUnmatched, "ACTION", (Icallback)uiSyncInteger);
    IupSetAttribute(actionListUnmatched, SYNCED_VALUE, (char*)&selectedActionUnmatched);

    // Setup lag time input (for matched packets)
    IupSetAttribute(lagTimeInput, "VISIBLECOLUMNS", "3");
    IupSetAttribute(lagTimeInput, "VALUE", STR(LAG_TIME_DEFAULT));
    IupSetCallback(lagTimeInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(lagTimeInput, SYNCED_VALUE, (char*)&lagTime);
    IupSetAttribute(lagTimeInput, INTEGER_MAX, "15000");
    IupSetAttribute(lagTimeInput, INTEGER_MIN, "0");

    // Setup lag time input (for non-matched packets - not shown in UI)
    IupSetAttribute(lagTimeInputUnmatched, "VISIBLECOLUMNS", "1");
    IupSetAttribute(lagTimeInputUnmatched, "VALUE", STR(LAG_TIME_DEFAULT));
    IupSetCallback(lagTimeInputUnmatched, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(lagTimeInputUnmatched, SYNCED_VALUE, (char*)&lagTimeUnmatched);
    IupSetAttribute(lagTimeInputUnmatched, INTEGER_MAX, "15000");
    IupSetAttribute(lagTimeInputUnmatched, INTEGER_MIN, "0");

    // Setup duplicate count input (for matched packets)
    IupSetAttribute(duplicateCountInput, "VISIBLECOLUMNS", "2");
    IupSetAttribute(duplicateCountInput, "VALUE", "2");
    IupSetCallback(duplicateCountInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(duplicateCountInput, SYNCED_VALUE, (char*)&duplicateCount);
    IupSetAttribute(duplicateCountInput, INTEGER_MAX, "50");
    IupSetAttribute(duplicateCountInput, INTEGER_MIN, "2");

    // Setup duplicate count input (for non-matched packets - not shown in UI)
    IupSetAttribute(duplicateCountInputUnmatched, "VISIBLECOLUMNS", "1");
    IupSetAttribute(duplicateCountInputUnmatched, "VALUE", "2");
    IupSetCallback(duplicateCountInputUnmatched, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(duplicateCountInputUnmatched, SYNCED_VALUE, (char*)&duplicateCountUnmatched);
    IupSetAttribute(duplicateCountInputUnmatched, INTEGER_MAX, "50");
    IupSetAttribute(duplicateCountInputUnmatched, INTEGER_MIN, "2");

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
        setFromParameter(actionListUnmatched, "VALUE", NAME"-action-unmatched");
        setFromParameter(lagTimeInputUnmatched, "VALUE", NAME"-lagtime-unmatched");
        setFromParameter(duplicateCountInputUnmatched, "VALUE", NAME"-dupcount-unmatched");
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
    
    LOG("length filter enabled: min=%d, max=%d, matched_action=%d, unmatched_action=%d, lagTime=%d, lagTimeUnmatched=%d, duplicateCount=%d, duplicateCountUnmatched=%d", 
        minLength, maxLength, selectedAction, selectedActionUnmatched, lagTime, lagTimeUnmatched, duplicateCount, duplicateCountUnmatched);
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

// Helper function to apply action to a packet
// Returns 1 if packet was processed (removed from queue), 0 if packet should continue to next
static short applyLengthAction(PacketNode **head, PacketNode *pac, LengthAction action, 
                               short lagTimeVal, short dupCountVal, const char *actionLabel) {
    UINT packetLength = pac->packetLen;
    
    switch (action) {
        case LENGTH_ACTION_NOTHING:
            // Pass through without modification
            return 0;
            
        case LENGTH_ACTION_DROP:
            LOG("dropped packet with length %u bytes (range: %d-%d), direction %s - %s",
                packetLength, minLength, maxLength, 
                pac->addr.Outbound ? "OUTBOUND" : "INBOUND", actionLabel);
            logPacketAction(pac, "LENGTH_DROP", "length");
            freeNode(popNode(pac));
            return 1;
            
        case LENGTH_ACTION_LAG:
            // Add packet to lag buffer with timestamp
            if (lagBufSize < KEEP_AT_MOST) {
                PacketNode *moved = popNode(pac);
                moved->timestamp = timeGetTime();
                insertAfter(moved, lagBufHead);
                ++lagBufSize;
                LOG("Added packet to lag buffer, size: %d - %s", lagBufSize, actionLabel);
                logPacketAction(moved, "LENGTH_LAG", "length");
            } else {
                // Buffer full, just pass through
                return 0;
            }
            return 1;
            
        case LENGTH_ACTION_DUPLICATE:
            {
                short copies = dupCountVal - 1;
                LOG("duplicating packet with length %u bytes, cloned additionally %d packets - %s", 
                    packetLength, copies, actionLabel);
                while (copies-- > 0) {
                    PacketNode *copy = createNode(pac->packet, pac->packetLen, &(pac->addr));
                    insertBefore(copy, pac); // must insertBefore or next packet is still pac
                    logPacketAction(copy, "LENGTH_DUPLICATE", "length");
                }
                logPacketAction(pac, "LENGTH_DUPLICATE", "length");
                *head = pac; // Update head to current packet for next iteration
                return 0;
            }
            
        case LENGTH_ACTION_RESET:
            // Send RST packet (only for TCP)
            LOG("sending RST for packet with length %u bytes - %s", packetLength, actionLabel);
            logPacketAction(pac, "LENGTH_RESET", "length");
            // RST handling would require additional logic similar to reset.c module
            // For now, just drop it
            freeNode(popNode(pac));
            return 1;
            
        case LENGTH_ACTION_OOO:
            // Out of order - reorder by moving to different position
            LOG("reordering packet with length %u bytes - %s", packetLength, actionLabel);
            logPacketAction(pac, "LENGTH_OOO", "length");
            // OOO handling would require additional logic similar to ood.c module
            // For now, just pass through
            return 0;
            
        default:
            return 0;
    }
}

static short lengthProcess(PacketNode *head, PacketNode* tail) {
    int processed = 0;
    DWORD currentTime = timeGetTime();
    
    // First, try to send any lagged packets that are due
    if (selectedAction == LENGTH_ACTION_LAG || selectedActionUnmatched == LENGTH_ACTION_LAG) {
        while (!isLagBufEmpty()) {
            PacketNode *pac = lagBufTail->prev;
            if (currentTime > pac->timestamp + lagTime) {
                insertAfter(popNode(lagBufTail->prev), head);
                --lagBufSize;
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
                // Packet matches the length filter, apply selected action for matched
                if (applyLengthAction(&head, pac, selectedAction, lagTime, duplicateCount, "MATCHED")) {
                    ++processed;
                } else {
                    head = head->next;
                }
            } else {
                // Packet doesn't match the length filter, apply selected action for non-matched
                if (applyLengthAction(&head, pac, selectedActionUnmatched, lagTimeUnmatched, duplicateCountUnmatched, "NON-MATCHED")) {
                    ++processed;
                } else {
                    head = head->next;
                }
            }
        } else {
            head = head->next;
        }
    }

    return processed > 0;
}

static void lengthCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    // Flush all buffered packets when closing down
    flushLagBuffer(tail);
    LOG("length filter disabled");
    endTimePeriod();
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