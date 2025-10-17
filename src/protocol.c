// protocol-specific filtering module
#include <stdlib.h>
#include <Windows.h>
#include "iup.h"
#include "common.h"
#define NAME "protocol"

// Common protocol definitions
#define PROTO_HTTP_PORT     80
#define PROTO_HTTPS_PORT    443
#define PROTO_DNS_PORT      53
#define PROTO_SSH_PORT      22
#define PROTO_FTP_PORT      21
#define PROTO_TELNET_PORT   23
#define PROTO_SMTP_PORT     25
#define PROTO_POP3_PORT     110
#define PROTO_IMAP_PORT     143
#define PROTO_SNMP_PORT     161

typedef enum {
    PROTO_FILTER_NONE = 0,
    PROTO_FILTER_HTTP = 1,
    PROTO_FILTER_HTTPS = 2,
    PROTO_FILTER_DNS = 4,
    PROTO_FILTER_SSH = 8,
    PROTO_FILTER_FTP = 16,
    PROTO_FILTER_EMAIL = 32,
    PROTO_FILTER_CUSTOM = 64
} ProtocolFilter;

static Ihandle *inboundCheckbox, *outboundCheckbox;
static Ihandle *httpCheckbox, *httpsCheckbox, *dnsCheckbox, *sshCheckbox;
static Ihandle *ftpCheckbox, *emailCheckbox;
static Ihandle *customPortInput, *chanceInput;

static volatile short protocolEnabled = 0,
    protocolInbound = 1, protocolOutbound = 1,
    filterHTTP = 0, filterHTTPS = 0, filterDNS = 0,
    filterSSH = 0, filterFTP = 0, filterEmail = 0,
    customPort = 8080,
    chance = 1000; // [0-10000] - 10% default


// Helper function to check if packet matches selected protocols
static BOOL matchesProtocolFilter(PacketNode* pac) {
    PWINDIVERT_TCPHDR tcpHeader = NULL;
    PWINDIVERT_UDPHDR udpHeader = NULL;
    UINT16 srcPort, dstPort;
    
    // Parse packet headers
    WinDivertHelperParsePacket(pac->packet, pac->packetLen, NULL, NULL, NULL, NULL, NULL,
                              &tcpHeader, &udpHeader, NULL, NULL, NULL, NULL);
    
    if (tcpHeader) {
        srcPort = ntohs(tcpHeader->SrcPort);
        dstPort = ntohs(tcpHeader->DstPort);
    } else if (udpHeader) {
        srcPort = ntohs(udpHeader->SrcPort);
        dstPort = ntohs(udpHeader->DstPort);
    } else {
        return FALSE; // Not TCP or UDP
    }
    
    // Check HTTP
    if (filterHTTP && (srcPort == PROTO_HTTP_PORT || dstPort == PROTO_HTTP_PORT)) {
        return TRUE;
    }
    
    // Check HTTPS
    if (filterHTTPS && (srcPort == PROTO_HTTPS_PORT || dstPort == PROTO_HTTPS_PORT)) {
        return TRUE;
    }
    
    // Check DNS
    if (filterDNS && (srcPort == PROTO_DNS_PORT || dstPort == PROTO_DNS_PORT)) {
        return TRUE;
    }
    
    // Check SSH
    if (filterSSH && (srcPort == PROTO_SSH_PORT || dstPort == PROTO_SSH_PORT)) {
        return TRUE;
    }
    
    // Check FTP
    if (filterFTP && (srcPort == PROTO_FTP_PORT || dstPort == PROTO_FTP_PORT)) {
        return TRUE;
    }
    
    // Check Email protocols
    if (filterEmail && 
        (srcPort == PROTO_SMTP_PORT || dstPort == PROTO_SMTP_PORT ||
         srcPort == PROTO_POP3_PORT || dstPort == PROTO_POP3_PORT ||
         srcPort == PROTO_IMAP_PORT || dstPort == PROTO_IMAP_PORT)) {
        return TRUE;
    }
    
    // Check custom port
    if (customPort > 0 && (srcPort == customPort || dstPort == customPort)) {
        return TRUE;
    }
    
    return FALSE;
}

// Helper function to get protocol name for logging
static const char* getProtocolName(UINT16 port) {
    switch (port) {
        case PROTO_HTTP_PORT: return "HTTP";
        case PROTO_HTTPS_PORT: return "HTTPS";
        case PROTO_DNS_PORT: return "DNS";
        case PROTO_SSH_PORT: return "SSH";
        case PROTO_FTP_PORT: return "FTP";
        case PROTO_SMTP_PORT: return "SMTP";
        case PROTO_POP3_PORT: return "POP3";
        case PROTO_IMAP_PORT: return "IMAP";
        default: return "Custom";
    }
}

static Ihandle* protocolSetupUI() {
    Ihandle *protocolControlsBox = IupVbox(
        IupHbox(
            inboundCheckbox = IupToggle("Inbound", NULL),
            outboundCheckbox = IupToggle("Outbound", NULL),
            IupLabel("Chance(%):"),
            chanceInput = IupText(NULL),
            NULL
        ),
        IupHbox(
            httpCheckbox = IupToggle("HTTP", NULL),
            httpsCheckbox = IupToggle("HTTPS", NULL),
            dnsCheckbox = IupToggle("DNS", NULL),
            sshCheckbox = IupToggle("SSH", NULL),
            NULL
        ),
        IupHbox(
            ftpCheckbox = IupToggle("FTP", NULL),
            emailCheckbox = IupToggle("Email", NULL),
            IupLabel("Custom Port:"),
            customPortInput = IupText(NULL),
            NULL
        ),
        NULL
    );

    // Setup chance input
    IupSetAttribute(chanceInput, "VISIBLECOLUMNS", "4");
    IupSetAttribute(chanceInput, "VALUE", "10.0");
    IupSetCallback(chanceInput, "VALUECHANGED_CB", uiSyncChance);
    IupSetAttribute(chanceInput, SYNCED_VALUE, (char*)&chance);

    // Setup custom port input
    IupSetAttribute(customPortInput, "VISIBLECOLUMNS", "6");
    IupSetAttribute(customPortInput, "VALUE", "8080");
    IupSetCallback(customPortInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(customPortInput, SYNCED_VALUE, (char*)&customPort);
    IupSetAttribute(customPortInput, INTEGER_MIN, "1");
    IupSetAttribute(customPortInput, INTEGER_MAX, "65535");

    // Setup protocol checkboxes
    IupSetCallback(httpCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(httpCheckbox, SYNCED_VALUE, (char*)&filterHTTP);
    
    IupSetCallback(httpsCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(httpsCheckbox, SYNCED_VALUE, (char*)&filterHTTPS);
    
    IupSetCallback(dnsCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(dnsCheckbox, SYNCED_VALUE, (char*)&filterDNS);
    
    IupSetCallback(sshCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(sshCheckbox, SYNCED_VALUE, (char*)&filterSSH);
    
    IupSetCallback(ftpCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(ftpCheckbox, SYNCED_VALUE, (char*)&filterFTP);
    
    IupSetCallback(emailCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(emailCheckbox, SYNCED_VALUE, (char*)&filterEmail);

    // Setup direction checkboxes
    IupSetCallback(inboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(inboundCheckbox, SYNCED_VALUE, (char*)&protocolInbound);
    IupSetCallback(outboundCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(outboundCheckbox, SYNCED_VALUE, (char*)&protocolOutbound);

    // Enable by default
    IupSetAttribute(inboundCheckbox, "VALUE", "ON");
    IupSetAttribute(outboundCheckbox, "VALUE", "ON");

    if (parameterized) {
        setFromParameter(inboundCheckbox, "VALUE", NAME"-inbound");
        setFromParameter(outboundCheckbox, "VALUE", NAME"-outbound");
        setFromParameter(chanceInput, "VALUE", NAME"-chance");
        setFromParameter(httpCheckbox, "VALUE", NAME"-http");
        setFromParameter(httpsCheckbox, "VALUE", NAME"-https");
        setFromParameter(dnsCheckbox, "VALUE", NAME"-dns");
        setFromParameter(sshCheckbox, "VALUE", NAME"-ssh");
        setFromParameter(ftpCheckbox, "VALUE", NAME"-ftp");
        setFromParameter(emailCheckbox, "VALUE", NAME"-email");
        setFromParameter(customPortInput, "VALUE", NAME"-port");
    }

    return protocolControlsBox;
}

static void protocolStartup() {
    LOG("protocol filter enabled: HTTP=%d, HTTPS=%d, DNS=%d, SSH=%d, FTP=%d, Email=%d, Custom=%d", 
        filterHTTP, filterHTTPS, filterDNS, filterSSH, filterFTP, filterEmail, customPort);
}

static void protocolCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
    LOG("protocol filter disabled");
}

static short protocolProcess(PacketNode *head, PacketNode* tail) {
    int processed = 0;
    
    while (head->next != tail) {
        PacketNode *pac = head->next;
        
        if (checkDirection(pac->addr.Outbound, protocolInbound, protocolOutbound)) {
            if (matchesProtocolFilter(pac) && calcChance(chance)) {
                PWINDIVERT_TCPHDR tcpHeader = NULL;
                PWINDIVERT_UDPHDR udpHeader = NULL;
                UINT16 port = 0;
                
                // Get port for logging
                WinDivertHelperParsePacket(pac->packet, pac->packetLen, NULL, NULL, NULL, NULL, NULL,
                                          &tcpHeader, &udpHeader, NULL, NULL, NULL, NULL);
                if (tcpHeader) {
                    port = pac->addr.Outbound ? ntohs(tcpHeader->DstPort) : ntohs(tcpHeader->SrcPort);
                } else if (udpHeader) {
                    port = pac->addr.Outbound ? ntohs(udpHeader->DstPort) : ntohs(udpHeader->SrcPort);
                }
                
                LOG("dropped %s packet on port %d with chance %.1f%%, direction %s",
                    getProtocolName(port), port, chance/100.0, 
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

Module protocolModule = {
    "Protocol Filter",
    NAME,
    (short*)&protocolEnabled,
    protocolSetupUI,
    protocolStartup,
    protocolCloseDown,
    protocolProcess,
    // runtime fields
    0, 0, NULL
};