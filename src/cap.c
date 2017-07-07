// bandwidth cap
#include <math.h>
#include "iup.h"
#include "common.h"
#define NAME "cap"
#define CAP_MIN "0.1"
#define CAP_MAX "2000000000.0"
#define KEEP_AT_MOST 5000
#define BUFFER_MAX_KB "100000"
#define BUFFER_MIN_KB "1"

#define ALLOW_SET_BUFFER_SIZE 0

static Ihandle *inboundCheckbox, *outboundCheckbox, *kpsInput, *kbBufSzInput;

static volatile short capEnabled = 0,
    capInbound = 1, capOutbound = 1;
static volatile long
    kps = (long)(32 / FIXED_EPSILON),
	kbBufSz = 200; // kb / second

static PacketNode capHeadNode = {0}, capTailNode = {0};
static PacketNode *bufHead = &capHeadNode, *bufTail = &capTailNode;
static int bufSize = 0;
static int bufSizeBytes = 0;
static DWORD capLastTick = 0;

static Ihandle* capSetupUI() {
    Ihandle *capControlsBox = IupHbox(
        inboundCheckbox = IupToggle("Inbound", NULL),
        outboundCheckbox = IupToggle("Outbound", NULL),
        IupLabel("Bandwidth Cap(kB/s):"),
        kpsInput = IupText(NULL),
#if ALLOW_SET_BUFFER_SIZE
        IupLabel("Buffer(kB):"),
		kbBufSzInput = IupText(NULL),
#endif
        NULL
        );

    IupSetCallback(inboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(inboundCheckbox, SYNCED_VALUE, (char*)&capInbound);
    IupSetCallback(outboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(outboundCheckbox, SYNCED_VALUE, (char*)&capOutbound);

    IupSetAttribute(kpsInput, "VISIBLECOLUMNS", "4");
    IupSetAttribute(kpsInput, "VALUE", "32.0");
    IupSetCallback(kpsInput, "VALUECHANGED_CB", (Icallback)uiSyncFixedInt);
    IupSetAttribute(kpsInput, SYNCED_VALUE, (char*)&kps);
    IupSetAttribute(kpsInput, FIXED_MAX, CAP_MAX);
    IupSetAttribute(kpsInput, FIXED_MIN, CAP_MIN);

#if ALLOW_SET_BUFFER_SIZE
    IupSetAttribute(kbBufSzInput, "VISIBLECOLUMNS", "4");
    IupSetAttribute(kbBufSzInput, "VALUE", "200");
    IupSetCallback(kbBufSzInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(kbBufSzInput, SYNCED_VALUE, (char*)&kbBufSz);
    IupSetAttribute(kbBufSzInput, INTEGER_MAX, BUFFER_MAX_KB);
    IupSetAttribute(kbBufSzInput, INTEGER_MIN, BUFFER_MIN_KB);
#endif

    // enable by default to avoid confusing
    IupSetAttribute(inboundCheckbox, "VALUE", "ON");
    IupSetAttribute(outboundCheckbox, "VALUE", "ON");

    if (parameterized) {
        setFromParameter(inboundCheckbox, "VALUE", NAME"-inbound");
        setFromParameter(outboundCheckbox, "VALUE", NAME"-outbound");
        setFromParameter(kpsInput, "VALUE", NAME"-kps");
#if ALLOW_SET_BUFFER_SIZE
		setFromParameter(kbBufSzInput, "VALUE", NAME"-buf-sz-kb");
#endif
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
		bufSizeBytes = 0;
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
	int capCnt = 0;
	int droppedCnt = 0;

	int maxBufSize = kps * 1024 / 4;

#if ALLOW_SET_BUFFER_SIZE
	maxBufSize = kbBufSz * 1024;
#endif

    // process buffered packets
    oldLast = tail->prev;
    while (!isBufEmpty()) {
        // TODO should check direction in buffer?
        // sends at least one from buffer or it would get stuck
        pac = bufTail->prev;
        totalBytes += pac->packetLen;

        if (totalBytes > bytesCapped) {
			totalBytes -= pac->packetLen;
            break;
        }   

        insertAfter(popNode(pac), oldLast);
        --bufSize;
		bufSizeBytes -= pac->packetLen;

        LOG("sending out packets of %d bytes", totalBytes);
    }

	if(tail->prev != oldLast || (tail->prev == head && isBufEmpty()))
	{
		LOG("kps delta: %d, val: %d, capped kps %.2f, capped at %d bytes", deltaTick, kps, kps * FIXED_EPSILON, bytesCapped);
		capLastTick = curTick;
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
            capped = TRUE;
            // buffer from pac to head 
			if (bufSize < KEEP_AT_MOST && bufSizeBytes + pac->packetLen <= maxBufSize) {
                pacTmp = pac->prev;
                insertAfter(popNode(pac), bufHead);
                ++bufSize;
				bufSizeBytes += pac->packetLen;
                ++capCnt;
                pac = pacTmp;
            }
			else
			{
				assert(pac != tail);
				++capCnt;
				++droppedCnt;
				pacTmp = pac->prev;
				popNode(pac);
				freeNode(pac);
				pac = pacTmp;
            }
        } else {
            pac = pac->prev;
        }
    }

	if(capped)
	{
		assert(pac == head);
		LOG("capping %d packets", capCnt);
		LOG("dropped %d packets", droppedCnt);
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