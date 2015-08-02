// tampering packet module
#include "iup.h"
#include "windivert.h"
#include "common.h"
#define NAME "tamper"

static Ihandle *inboundCheckbox, *outboundCheckbox, *chanceInput, *checksumCheckbox;

static volatile short tamperEnabled = 0,
    tamperInbound = 1,
    tamperOutbound = 1,
    chance = 1000, // [0 - 10000]
    doChecksum = 1; // recompute checksum after after tampering

static Ihandle* tamperSetupUI() {
    Ihandle *dupControlsBox = IupHbox(
        checksumCheckbox = IupToggle("Redo Checksum", NULL),
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
    IupSetAttribute(inboundCheckbox, SYNCED_VALUE, (char*)&tamperInbound);
    IupSetCallback(outboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(outboundCheckbox, SYNCED_VALUE, (char*)&tamperOutbound);
    // sync doChecksum
    IupSetCallback(checksumCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(checksumCheckbox, SYNCED_VALUE, (char*)&doChecksum);

    // enable by default to avoid confusing
    IupSetAttribute(inboundCheckbox, "VALUE", "ON");
    IupSetAttribute(outboundCheckbox, "VALUE", "ON");
    IupSetAttribute(checksumCheckbox, "VALUE", "ON");

    if (parameterized) {
        setFromParameter(inboundCheckbox, "VALUE", NAME"-inbound");
        setFromParameter(outboundCheckbox, "VALUE", NAME"-outbound");
        setFromParameter(chanceInput, "VALUE", NAME"-chance");
        setFromParameter(checksumCheckbox, "VALUE", NAME"-checksum");
    }

    return dupControlsBox;
}

// patterns covers every bit
#define PATTERN_CNT 8
static char patterns[] = {
    0x64,
    0x13,
    0x88,

    0x40,
    0x1F,
    0xA0,

    0xAA,
    0x55
};
static int patIx; // put this here to give a more random results

static void tamperStartup() {
    LOG("tamper enabled");
    patIx = 0;
}

static void tamperCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
    LOG("tamper disabled");
}

static INLINE_FUNCTION void tamper_buf(char *buf, UINT len) {
    UINT ix;
    for (ix = 0; ix < len; ++ix) {
        buf[ix] ^= patterns[patIx++ & 0x7];
    }
}

static short tamperProcess(PacketNode *head, PacketNode *tail) {
    short tampered = FALSE;
    PacketNode *pac = head->next;
    while (pac != tail) {
        if (checkDirection(pac->addr.Direction, tamperInbound, tamperOutbound)
            && calcChance(chance)) {
            char *data = NULL;
            UINT dataLen = 0;
            if (WinDivertHelperParsePacket(pac->packet, pac->packetLen, NULL, NULL, NULL,
                NULL, NULL, NULL, (PVOID*)&data, &dataLen) 
                && data != NULL && dataLen != 0) {
                // try to tamper the central part of the packet,
                // since common packets put their checksum at head or tail
                if (dataLen <= 4) {
                    // for short packet just tamper it all
                    tamper_buf(data, dataLen);
                    LOG("tampered w/ chance %.1f, dochecksum: %d, short packet changed all", chance/100.0, doChecksum);
                } else {
                    // for longer ones process 1/4 of the lens start somewhere in the middle
                    UINT len = dataLen;
                    UINT len_d4 = len / 4;
                    tamper_buf(data + len/2 - len_d4/2 + 1, len_d4);
                    LOG("tampered w/ chance %.1f, dochecksum: %d, changing %d bytes out of %u", chance/100.0, doChecksum, len_d4, len);
                }
                // FIXME checksum seems to have some problem
                if (doChecksum) {
                    WinDivertHelperCalcChecksums(pac->packet, pac->packetLen, 0);
                }
                tampered = TRUE;
            }

        }
        pac = pac->next;
    }
    return tampered;
}

Module tamperModule = {
    "Tamper",
    NAME,
    (short*)&tamperEnabled,
    tamperSetupUI,
    tamperStartup,
    tamperCloseDown,
    tamperProcess,
    // runtime fields
    0, 0, NULL
};
