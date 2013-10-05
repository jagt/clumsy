// out of order arrange packets module
#include <assert.h>
#include "iup.h"
#include "common.h"
// keep a picked packet at most for KEEP_TURNS_MAX steps, or if there's no following
// one it would just to be sended
#define KEEP_TURNS_MAX 10 

static Ihandle *inboundCheckbox, *outboundCheckbox, *chanceInput;

static volatile short oodEnabled = 0,
    oodInbound = 0, oodOutbound = 0,
    chance = 100; // [0-1000]
static PacketNode *oodPacket = NULL;
static int giveUpCnt;

static Ihandle *oodSetupUI() {
    Ihandle *oodControlsBox = IupHbox(
        inboundCheckbox = IupToggle("Ooo Inbound", NULL),
        outboundCheckbox = IupToggle("Ooo Outbound", NULL),
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

    return oodControlsBox;
}

static void oodStartUp() {
    LOG("ood enabled");
    giveUpCnt = KEEP_TURNS_MAX;
    // assert on the issue that repeatly enable/disable abort the program
    assert(oodPacket == NULL);
}

static void oodCloseDown(PacketNode *head, PacketNode *tail) {
    LOG("ood disabled");
    if (oodPacket != NULL) {
        insertAfter(oodPacket, head);
        oodPacket = NULL; // ! need to empty the ood packet
    }
}

static void oodProcess(PacketNode *head, PacketNode *tail) {
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
            if ((oodInbound && IS_INBOUND(pac->addr.Direction)
                || oodOutbound && IS_OUTBOUND(pac->addr.Direction)
                ) && calcChance(chance)) {
                oodPacket = popNode(pac);
                LOG("Ooo picked packet w/ chance %.1f% , direction %s", chance/10.0, BOUND_TEXT(pac->addr.Direction));
            }
        } else {
            // pass
            // since ood is the second module after drop, it'll see at most one packet
            // TODO if there's more packets, do the ood without picking out packets
        }
    }
}

Module oodModule = {
    "Out of order",
    (short*)&oodEnabled,
    oodSetupUI,
    oodStartUp,
    oodCloseDown,
    oodProcess
};