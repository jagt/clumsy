// TLS-aware traffic handler module
#include <stdlib.h>
#include <Windows.h>
#include "iup.h"
#include "common.h"
#define NAME "tls"

// TLS record types (RFC 5246)
#define TLS_RECORD_CHANGE_CIPHER_SPEC   20
#define TLS_RECORD_ALERT               21
#define TLS_RECORD_HANDSHAKE           22
#define TLS_RECORD_APPLICATION_DATA    23

// TLS handshake types
#define TLS_HANDSHAKE_CLIENT_HELLO      1
#define TLS_HANDSHAKE_SERVER_HELLO      2
#define TLS_HANDSHAKE_CERTIFICATE      11
#define TLS_HANDSHAKE_SERVER_KEY_EXCHANGE 12
#define TLS_HANDSHAKE_CERTIFICATE_REQUEST 13
#define TLS_HANDSHAKE_SERVER_HELLO_DONE 14
#define TLS_HANDSHAKE_CERTIFICATE_VERIFY 15
#define TLS_HANDSHAKE_CLIENT_KEY_EXCHANGE 16
#define TLS_HANDSHAKE_FINISHED         20

typedef struct {
    UINT8 contentType;
    UINT16 version;
    UINT16 length;
} TLSRecordHeader;

static Ihandle *inboundCheckbox, *outboundCheckbox;
static Ihandle *handshakeProtectionInput, *dataLossRateInput;
static Ihandle *sequenceAwareCheckbox, *retransmissionDelayInput;

static volatile short tlsEnabled = 0,
    tlsInbound = 1, tlsOutbound = 1,
    sequenceAware = 1,
    handshakeProtection = 95, // Protect handshake packets with 95% probability
    dataLossRate = 500,       // 5% loss rate for application data
    retransmissionDelay = 200; // 200ms delay for retransmissions


// Helper function to parse TLS record from packet data
static BOOL parseTLSRecord(char* packet, UINT packetLen, TLSRecordHeader* tlsHeader, char** tlsPayload) {
    PWINDIVERT_IPHDR ipHeader = NULL;
    PWINDIVERT_TCPHDR tcpHeader = NULL;
    char* data = NULL;
    UINT dataLen = 0;
    
    // Parse packet to get TCP payload
    if (!WinDivertHelperParsePacket(packet, packetLen, &ipHeader, NULL, NULL, NULL, NULL,
                                   &tcpHeader, NULL, (PVOID*)&data, &dataLen, NULL, NULL)) {
        return FALSE;
    }
    
    // Check if this is TCP traffic
    if (!tcpHeader || !data || dataLen < sizeof(TLSRecordHeader)) {
        return FALSE;
    }
    
    // Parse TLS record header
    tlsHeader->contentType = data[0];
    tlsHeader->version = (data[1] << 8) | data[2];
    tlsHeader->length = (data[3] << 8) | data[4];
    
    // Validate TLS record
    if (tlsHeader->contentType < TLS_RECORD_CHANGE_CIPHER_SPEC || 
        tlsHeader->contentType > TLS_RECORD_APPLICATION_DATA ||
        tlsHeader->length + 5 > dataLen) {
        return FALSE;
    }
    
    *tlsPayload = data + 5; // Skip TLS record header
    return TRUE;
}

// Check if this is a TLS handshake packet
static BOOL isTLSHandshakePacket(TLSRecordHeader* tlsHeader, char* tlsPayload) {
    if (tlsHeader->contentType == TLS_RECORD_HANDSHAKE && tlsHeader->length > 0) {
        UINT8 handshakeType = tlsPayload[0];
        return (handshakeType >= TLS_HANDSHAKE_CLIENT_HELLO && 
                handshakeType <= TLS_HANDSHAKE_FINISHED);
    }
    return FALSE;
}

// Check if packet is on TLS port (443 or other HTTPS ports)
static BOOL isTLSPort(PWINDIVERT_TCPHDR tcpHeader) {
    UINT16 srcPort = ntohs(tcpHeader->SrcPort);
    UINT16 dstPort = ntohs(tcpHeader->DstPort);
    
    // Common TLS ports
    return (srcPort == 443 || dstPort == 443 ||    // HTTPS
            srcPort == 8443 || dstPort == 8443 ||  // Alternative HTTPS
            srcPort == 993 || dstPort == 993 ||    // IMAPS
            srcPort == 995 || dstPort == 995);     // POP3S
}

static Ihandle* tlsSetupUI() {
    Ihandle *tlsControlsBox = IupVbox(
        IupHbox(
            inboundCheckbox = IupToggle("In", NULL),
            outboundCheckbox = IupToggle("Out", NULL),
            sequenceAwareCheckbox = IupToggle("Seq Aware", NULL),
            NULL
        ),
        IupHbox(
            IupLabel("Handshake:"),
            handshakeProtectionInput = IupText(NULL),
            IupLabel("%"),
            IupLabel("Data Loss:"),
            dataLossRateInput = IupText(NULL),
            IupLabel("%"),
            NULL
        ),
        IupHbox(
            IupLabel("Delay:"),
            retransmissionDelayInput = IupText(NULL),
            IupLabel("ms"),
            NULL
        ),
        NULL
    );

    // Setup handshake protection input
    IupSetAttribute(handshakeProtectionInput, "VISIBLECOLUMNS", "3");
    IupSetAttribute(handshakeProtectionInput, "VALUE", "95.0");
    IupSetCallback(handshakeProtectionInput, "VALUECHANGED_CB", uiSyncChance);
    IupSetAttribute(handshakeProtectionInput, SYNCED_VALUE, (char*)&handshakeProtection);

    // Setup data loss rate input  
    IupSetAttribute(dataLossRateInput, "VISIBLECOLUMNS", "3");
    IupSetAttribute(dataLossRateInput, "VALUE", "5.0");
    IupSetCallback(dataLossRateInput, "VALUECHANGED_CB", uiSyncChance);
    IupSetAttribute(dataLossRateInput, SYNCED_VALUE, (char*)&dataLossRate);

    // Setup retransmission delay input
    IupSetAttribute(retransmissionDelayInput, "VISIBLECOLUMNS", "4");
    IupSetAttribute(retransmissionDelayInput, "VALUE", "200");
    IupSetCallback(retransmissionDelayInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(retransmissionDelayInput, SYNCED_VALUE, (char*)&retransmissionDelay);
    IupSetAttribute(retransmissionDelayInput, INTEGER_MIN, "0");
    IupSetAttribute(retransmissionDelayInput, INTEGER_MAX, "5000");

    // Setup direction checkboxes
    IupSetCallback(inboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(inboundCheckbox, SYNCED_VALUE, (char*)&tlsInbound);
    IupSetCallback(outboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(outboundCheckbox, SYNCED_VALUE, (char*)&tlsOutbound);

    // Setup sequence aware checkbox
    IupSetCallback(sequenceAwareCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(sequenceAwareCheckbox, SYNCED_VALUE, (char*)&sequenceAware);

    // Enable by default
    IupSetAttribute(inboundCheckbox, "VALUE", "ON");
    IupSetAttribute(outboundCheckbox, "VALUE", "ON");
    IupSetAttribute(sequenceAwareCheckbox, "VALUE", "ON");

    if (parameterized) {
        setFromParameter(inboundCheckbox, "VALUE", NAME"-inbound");
        setFromParameter(outboundCheckbox, "VALUE", NAME"-outbound");
        setFromParameter(sequenceAwareCheckbox, "VALUE", NAME"-sequence");
        setFromParameter(handshakeProtectionInput, "VALUE", NAME"-handshake");
        setFromParameter(dataLossRateInput, "VALUE", NAME"-data");
        setFromParameter(retransmissionDelayInput, "VALUE", NAME"-delay");
    }

    return tlsControlsBox;
}

static void tlsStartup() {
    LOG("TLS handler enabled: handshake protection=%d%%, data loss=%d%%, delay=%dms", 
        handshakeProtection/100, dataLossRate/100, retransmissionDelay);
}

static void tlsCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
    LOG("TLS handler disabled");
}

static short tlsProcess(PacketNode *head, PacketNode* tail) {
    int processed = 0;
    
    while (head->next != tail) {
        PacketNode *pac = head->next;
        
        if (checkDirection(pac->addr.Outbound, tlsInbound, tlsOutbound)) {
            PWINDIVERT_TCPHDR tcpHeader = NULL;
            TLSRecordHeader tlsHeader;
            char* tlsPayload = NULL;
            BOOL shouldDrop = FALSE;
            
            // Parse TCP header first
            WinDivertHelperParsePacket(pac->packet, pac->packetLen, NULL, NULL, NULL, NULL, NULL,
                                     &tcpHeader, NULL, NULL, NULL, NULL, NULL);
            
            // Only process TLS traffic
            if (tcpHeader && isTLSPort(tcpHeader)) {
                if (parseTLSRecord(pac->packet, pac->packetLen, &tlsHeader, &tlsPayload)) {
                    // TLS traffic detected
                    if (isTLSHandshakePacket(&tlsHeader, tlsPayload)) {
                        // Handshake packet - apply protection
                        if (!calcChance(handshakeProtection)) {
                            shouldDrop = TRUE;
                            LOG("Dropped TLS handshake packet (protection: %d%%), direction %s",
                                handshakeProtection/100, pac->addr.Outbound ? "OUTBOUND" : "INBOUND");
                        }
                    } else if (tlsHeader.contentType == TLS_RECORD_APPLICATION_DATA) {
                        // Application data - apply normal loss rate
                        if (calcChance(dataLossRate)) {
                            shouldDrop = TRUE;
                            LOG("Dropped TLS application data (loss rate: %d%%), direction %s",
                                dataLossRate/100, pac->addr.Outbound ? "OUTBOUND" : "INBOUND");
                        }
                    }
                    
                    if (shouldDrop) {
                        freeNode(popNode(pac));
                        ++processed;
                        continue;
                    }
                    
                    // Add artificial delay for retransmissions if enabled
                    if (retransmissionDelay > 0 && sequenceAware) {
                        // Check for potential retransmission (simplified heuristic)
                        if (tlsHeader.contentType == TLS_RECORD_APPLICATION_DATA) {
                            // Add to lag processing if lag module exists
                            // For now, just pass through with logging
                            LOG("TLS packet processed with %dms delay consideration", retransmissionDelay);
                        }
                    }
                }
            }
        }
        
        head = head->next;
    }

    return processed > 0;
}

Module tlsModule = {
    "TLS Handler",
    NAME,
    (short*)&tlsEnabled,
    tlsSetupUI,
    tlsStartup,
    tlsCloseDown,
    tlsProcess,
    // runtime fields
    0, 0, NULL
};