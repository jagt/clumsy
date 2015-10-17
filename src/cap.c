// bandwidth cap
#include <math.h>
#include "iup.h"
#include "common.h"
#define NAME "cap"
#define CAP_MIN "0.1"
#define CAP_MAX "320.0" // TODO CAP_MAX actually can't be larger than 2**15...
#define KEEP_AT_MOST 5000

static Ihandle *inboundCheckbox, *outboundCheckbox, *kpsInput;

static volatile short capEnabled = 0,
    capInbound = 1, capOutbound = 1,
    kps = (short)(32 / FIXED_EPSILON); // kb / second

static PacketNode capHeadNode = {0}, capTailNode = {0};
static PacketNode *bufHead = &capHeadNode, *bufTail = &capTailNode;
static int bufSize = 0;
static DWORD capLastTick = 0;

static Ihandle* capSetupUI() {
    Ihandle *capControlsBox = IupHbox(
        inboundCheckbox = IupToggle("Inbound", NULL),
        outboundCheckbox = IupToggle("Outbound", NULL),
        IupLabel("Bandwidth Cap(kb/s):"),
        kpsInput = IupText(NULL),
        NULL
        );

    IupSetCallback(inboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(inboundCheckbox, SYNCED_VALUE, (char*)&capInbound);
    IupSetCallback(outboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(outboundCheckbox, SYNCED_VALUE, (char*)&capOutbound);

    IupSetAttribute(kpsInput, "VISIBLECOLUMNS", "4");
    IupSetAttribute(kpsInput, "VALUE", "32.0");
    IupSetCallback(kpsInput, "VALUECHANGED_CB", (Icallback)uiSyncFixed);
    IupSetAttribute(kpsInput, SYNCED_VALUE, (char*)&kps);
    IupSetAttribute(kpsInput, FIXED_MAX, CAP_MAX);
    IupSetAttribute(kpsInput, FIXED_MIN, CAP_MIN);

    // enable by default to avoid confusing
    IupSetAttribute(inboundCheckbox, "VALUE", "ON");
    IupSetAttribute(outboundCheckbox, "VALUE", "ON");

    if (parameterized) {
        setFromParameter(inboundCheckbox, "VALUE", NAME"-inbound");
        setFromParameter(outboundCheckbox, "VALUE", NAME"-outbound");
        setFromParameter(kpsInput, "VALUE", NAME"-kps");
    }

    return capControlsBox;
}

// TODO these are exactly the same as throttle ones, try move them into packet.c
static INLINE_FUNCTION short isBufEmpty() {
    short ret = bufHead->next == bufTail;
    if (ret) assert(bufSize == 0);
    return ret;
}

static void clearBufPackets(PacketNode *tail) {
    PacketNode *oldLast = tail->prev;
    LOG("Cap end, send all %d packets. Buffer at max: %s", bufSize, bufSize);
    while (!isBufEmpty()) {
        insertAfter(popNode(bufTail->prev), oldLast);
        --bufSize;
    }
}

static void capStartUp() {
    if (bufHead->next == NULL && bufTail->next == NULL) {
        bufHead->next = bufTail;
        bufTail->prev = bufHead;
        bufSize = 0;
    } else {
        assert(isBufEmpty());
    }

    startTimePeriod();
    capLastTick = timeGetTime();
}

static void capCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);

    clearBufPackets(tail);
    endTimePeriod();
}

static short capProcess(PacketNode *head, PacketNode *tail) {
    short capped = FALSE;
    PacketNode *pac, *pacTmp, *oldLast;
    DWORD curTick = timeGetTime();
    DWORD deltaTick = curTick - capLastTick;
    int bytesCapped = (int)(deltaTick * 0.001 * kps * FIXED_EPSILON * 1024);
    int totalBytes = 0;
    LOG("kps val: %d, capped kps %.2f, capped at %d bytes", kps, kps * FIXED_EPSILON, bytesCapped);
    capLastTick = curTick;


    // process buffered packets
    oldLast = tail->prev;
    while (!isBufEmpty()) {
        // TODO should check direction in buffer?
        // sends at least one from buffer or it would get stuck
        pac = bufTail->prev;
        totalBytes += pac->packetLen;
        insertAfter(popNode(pac), oldLast);
        --bufSize;

        LOG("sending out packets of %d bytes", totalBytes);

        if (totalBytes > bytesCapped) {
            break;
        }   
    }

    // process live packets
    pac = oldLast;
    while (pac != head) {
        if (!checkDirection(pac->addr.Direction, capInbound, capOutbound)) {
            pac = pac->prev;
            continue;
        }

        // live packets can all be kept
        totalBytes += pac->packetLen;

        if (totalBytes > bytesCapped) {
            int capCnt = 0;
            capped = TRUE;
            // buffer from pac to head 
            while (bufSize < KEEP_AT_MOST && pac != head) {
                pacTmp = pac->prev;
                insertAfter(popNode(pac), bufHead);
                ++bufSize;
                ++capCnt;
                pac = pacTmp;
            }

            if (pac != head) {
                LOG("! hitting cap max, dropping all remaining");
                while (pac != head) {
                    pacTmp = pac->prev;
                    freeNode(pac);
                    pac = pacTmp;
                }
            }
            assert(pac == head);
            LOG("capping %d packets", capCnt);
            break;
        } else {
            pac = pac->prev;
        }
    }

    return capped;
}

Module capModule = {
    "Cap",
    NAME,
    (short*)&capEnabled,
    capSetupUI,
    capStartUp,
    capCloseDown,
    capProcess,
    // runtime fields
    0, 0, NULL
};