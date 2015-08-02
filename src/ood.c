// out of order arrange packets module
#include "iup.h"
#include "common.h"
#define NAME "ood"
// keep a picked packet at most for KEEP_TURNS_MAX steps, or if there's no following
// one, it will just be sent
#define KEEP_TURNS_MAX 10 

static Ihandle *inboundCheckbox, *outboundCheckbox, *chanceInput;

static volatile short oodEnabled = 0,
    oodInbound = 1, oodOutbound = 1,
    chance = 1000; // [0-10000]
static PacketNode *oodPacket = NULL;
static int giveUpCnt;

static Ihandle *oodSetupUI() {
    Ihandle *oodControlsBox = IupHbox(
        inboundCheckbox = IupToggle("Inbound", NULL),
        outboundCheckbox = IupToggle("Outbound", NULL),
        IupLabel("Chance(%):"),
        chanceInput = IupText(NULL),
        NULL
    );

    IupSetAttribute(chanceInput, "VISIBLECOLUMNS", "4");
    IupSetAttribute(chanceInput, "VALUE", "10.0");
    IupSetCallback(chanceInput, "VALUECHANGED_CB", uiSyncChance);
    IupSetAttribute(chanceInput, SYNCED_VALUE, (char*)&chance);
    IupSetCallback(inboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(inboundCheckbox, SYNCED_VALUE, (char*)&oodInbound);
    IupSetCallback(outboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(outboundCheckbox, SYNCED_VALUE, (char*)&oodOutbound);

    // enable by default to avoid confusing
    IupSetAttribute(inboundCheckbox, "VALUE", "ON");
    IupSetAttribute(outboundCheckbox, "VALUE", "ON");

    if (parameterized) {
        setFromParameter(inboundCheckbox, "VALUE", NAME"-inbound");
        setFromParameter(outboundCheckbox, "VALUE", NAME"-outbound");
        setFromParameter(chanceInput, "VALUE", NAME"-chance");
    }

    return oodControlsBox;
}

static void oodStartUp() {
    LOG("ood enabled");
    giveUpCnt = KEEP_TURNS_MAX;
    // assert on the issue that repeatly enable/disable abort the program
    assert(oodPacket == NULL);
}

static void oodCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(tail);
    LOG("ood disabled");
    if (oodPacket != NULL) {
        insertAfter(oodPacket, head);
        oodPacket = NULL; // ! need to empty the ood packet
    }
}

// find the next packet fits the direction check or null
static PacketNode* nextCorrectDirectionNode(PacketNode *p) {
    if (p == NULL) {
        return NULL;
    }

    do {
        p = p->next;
    } while (p->next != NULL && !checkDirection(p->addr.Direction, oodInbound, oodOutbound));

    return p->next == NULL ? NULL : p;
}

// not really perfect swap since it assumes a is before b
static void swapNode(PacketNode *a, PacketNode *b) {
    assert(a->prev && a->next && b->prev && b->next); // not accidentally swapping head/tail
    assert(a != b); // treat swap self as error here since we shouldn't really be doing it
    if (a->next == b) {
        // adjacent nodes need special care
        a->prev->next = b;
        b->next->prev = a;
        a->next = b->next;
        b->prev = a->prev;
        a->prev = b;
        b->next = a;
    } else {
        PacketNode *pa = a->prev,
                   *na = a->next,
                   *pb = b->prev,
                   *nb = b->next;
        pa->next = na->prev = b;
        b->prev = pa;
        b->next = na;
        pb->next = nb->prev = a;
        a->prev = pb;
        a->next = nb;
    }
}

static short oodProcess(PacketNode *head, PacketNode *tail) {
    if (oodPacket != NULL) {
        if (!isListEmpty() || --giveUpCnt == 0) {
            LOG("Ooo sent direction %s, is giveup %s", BOUND_TEXT(oodPacket->addr.Direction), giveUpCnt ? "NO" : "YES");
            insertAfter(oodPacket, head);
            oodPacket = NULL;
            giveUpCnt = KEEP_TURNS_MAX;
        } // skip picking packets when having oodPacket already
    } else if (!isListEmpty()) {
        PacketNode *pac = head->next;
        if (pac->next == tail) {
            // only contains a single packet, then pick it out and insert later
            if (checkDirection(pac->addr.Direction, oodInbound, oodOutbound) && calcChance(chance)) {
                oodPacket = popNode(pac);
                LOG("Ooo picked packet w/ chance %.1f%%, direction %s", chance/100.0, BOUND_TEXT(pac->addr.Direction));
                return TRUE;
            }
        } else if (calcChance(chance)) {
            // since there's already multiple packets in the queue, do a reorder will be enough
            PacketNode *first = head, *second;
            do {
                first = nextCorrectDirectionNode(first);
                second = nextCorrectDirectionNode(first);
                // calculate chance per swap
                if (first && second && calcChance(chance)) {
                    swapNode(first, second);
                    LOG("Multiple packets OOD swapping");
                } else {
                    // move forward first to progress
                    first = second;
                }
            } while (first && second);
            return TRUE;
        }
    }

    return FALSE;
}

Module oodModule = {
    "Out of order",
    NAME,
    (short*)&oodEnabled,
    oodSetupUI,
    oodStartUp,
    oodCloseDown,
    oodProcess,
    // runtime fields
    0, 0, NULL
};