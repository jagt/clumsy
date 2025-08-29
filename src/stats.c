// real-time statistics and monitoring module
#include <stdlib.h>
#include <Windows.h>
#include "iup.h"
#include "common.h"
#define NAME "stats"

// Statistics structure
typedef struct {
    DWORD startTime;
    DWORD lastUpdateTime;
    
    // Packet counters
    UINT64 totalPackets;
    UINT64 inboundPackets;
    UINT64 outboundPackets;
    UINT64 droppedPackets;
    UINT64 modifiedPackets;
    
    // Byte counters
    UINT64 totalBytes;
    UINT64 inboundBytes;
    UINT64 outboundBytes;
    UINT64 droppedBytes;
    
    // Protocol breakdown
    UINT64 tcpPackets;
    UINT64 udpPackets;
    UINT64 icmpPackets;
    UINT64 otherPackets;
    
    // Performance metrics
    UINT32 minLatency;
    UINT32 maxLatency;
    UINT64 totalLatency;
    UINT32 avgLatency;
    
    // Current rates (packets/sec, bytes/sec)
    UINT32 packetRate;
    UINT32 byteRate;
} NetworkStatistics;

static Ihandle *enabledCheckbox, *statsDisplay, *resetButton;

static volatile short statsEnabled = 0;
static volatile short displayMode = 1; // Always show detailed view
static NetworkStatistics stats = {0};
static CRITICAL_SECTION statsMutex;

// Helper function to update statistics
void updateStatistics(PacketNode* pac, BOOL wasDropped, BOOL wasModified) {
    EnterCriticalSection(&statsMutex);
    
    DWORD currentTime = GetTickCount();
    if (stats.startTime == 0) {
        stats.startTime = currentTime;
    }
    
    // Update packet counters
    stats.totalPackets++;
    if (pac->addr.Outbound) {
        stats.outboundPackets++;
    } else {
        stats.inboundPackets++;
    }
    
    if (wasDropped) {
        stats.droppedPackets++;
        stats.droppedBytes += pac->packetLen;
    } else {
        stats.totalBytes += pac->packetLen;
        if (pac->addr.Outbound) {
            stats.outboundBytes += pac->packetLen;
        } else {
            stats.inboundBytes += pac->packetLen;
        }
    }
    
    if (wasModified) {
        stats.modifiedPackets++;
    }
    
    // Update protocol breakdown
    PWINDIVERT_IPHDR ipHeader = NULL;
    PWINDIVERT_TCPHDR tcpHeader = NULL;
    PWINDIVERT_UDPHDR udpHeader = NULL;
    PWINDIVERT_ICMPHDR icmpHeader = NULL;
    
    WinDivertHelperParsePacket(pac->packet, pac->packetLen, &ipHeader, NULL, NULL,
                              &icmpHeader, NULL, &tcpHeader, &udpHeader, NULL, NULL, NULL, NULL);
    
    if (tcpHeader) {
        stats.tcpPackets++;
    } else if (udpHeader) {
        stats.udpPackets++;
    } else if (icmpHeader) {
        stats.icmpPackets++;
    } else {
        stats.otherPackets++;
    }
    
    // Calculate rates (update every second)
    if (currentTime - stats.lastUpdateTime >= 1000) {
        DWORD timeDiff = currentTime - stats.lastUpdateTime;
        if (timeDiff > 0) {
            stats.packetRate = (UINT32)((stats.totalPackets * 1000) / (currentTime - stats.startTime));
            stats.byteRate = (UINT32)((stats.totalBytes * 1000) / (currentTime - stats.startTime));
        }
        stats.lastUpdateTime = currentTime;
    }
    
    LeaveCriticalSection(&statsMutex);
}

// Format statistics for display - show all information on one panel
static void formatStatistics(char* buffer, size_t bufferSize) {
    EnterCriticalSection(&statsMutex);
    
    DWORD runtime = (GetTickCount() - stats.startTime) / 1000; // Convert to seconds
    if (runtime == 0) runtime = 1; // Avoid division by zero
    
    // Show all statistics in a comprehensive format
    snprintf(buffer, bufferSize,
        "Runtime: %dm %ds | Total Packets: %I64u (Dropped: %I64u, Modified: %I64u)\n"
        "Inbound: %I64u pkts (%I64u MB) | Outbound: %I64u pkts (%I64u MB)\n"
        "Protocols - TCP: %I64u | UDP: %I64u | ICMP: %I64u | Other: %I64u\n"
        "Current Rate: %u pps (%u kBps) | Drop Rate: %.2f%%",
        (int)(runtime / 60), (int)(runtime % 60),
        stats.totalPackets, stats.droppedPackets, stats.modifiedPackets,
        stats.inboundPackets, stats.inboundBytes / (1024 * 1024),
        stats.outboundPackets, stats.outboundBytes / (1024 * 1024),
        stats.tcpPackets, stats.udpPackets, stats.icmpPackets, stats.otherPackets,
        stats.packetRate, stats.byteRate / 1024,
        stats.totalPackets > 0 ? (stats.droppedPackets * 100.0) / stats.totalPackets : 0.0);
    
    LeaveCriticalSection(&statsMutex);
}

// Reset statistics
static int resetStatsCallback(Ihandle *ih) {
    UNREFERENCED_PARAMETER(ih);
    
    EnterCriticalSection(&statsMutex);
    memset(&stats, 0, sizeof(NetworkStatistics));
    stats.startTime = GetTickCount();
    stats.lastUpdateTime = stats.startTime;
    stats.minLatency = UINT32_MAX;
    LeaveCriticalSection(&statsMutex);
    
    LOG("Statistics reset");
    return IUP_DEFAULT;
}



static Ihandle* statsSetupUI() {
    Ihandle *statsControlsBox = IupVbox(
        IupHbox(
            enabledCheckbox = IupToggle("Enable Statistics", NULL),
            resetButton = IupButton("Reset", NULL),
            NULL
        ),
        statsDisplay = IupText(NULL),
        NULL
    );

    // Setup reset button
    IupSetCallback(resetButton, "ACTION", (Icallback)resetStatsCallback);
    IupSetAttribute(resetButton, "PADDING", "4x");

    // Setup enabled checkbox
    IupSetCallback(enabledCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(enabledCheckbox, SYNCED_VALUE, (char*)&statsEnabled);

    // Setup display text area
    IupSetAttribute(statsDisplay, "MULTILINE", "YES");
    IupSetAttribute(statsDisplay, "READONLY", "YES");
    IupSetAttribute(statsDisplay, "EXPAND", "YES");
    IupSetAttribute(statsDisplay, "SIZE", "400x200");
    IupSetAttribute(statsDisplay, "FONT", "Courier, 9");
    IupSetAttribute(statsDisplay, "VALUE", "Statistics will appear here...");

    if (parameterized) {
        setFromParameter(enabledCheckbox, "VALUE", NAME"-enabled");
    }

    return statsControlsBox;
}

static void statsStartup() {
    InitializeCriticalSection(&statsMutex);
    memset(&stats, 0, sizeof(NetworkStatistics));
    stats.startTime = GetTickCount();
    stats.lastUpdateTime = stats.startTime;
    stats.minLatency = UINT32_MAX;
    LOG("statistics monitoring enabled");
}

static void statsCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
    DeleteCriticalSection(&statsMutex);
    LOG("statistics monitoring disabled");
}

static short statsProcess(PacketNode *head, PacketNode* tail) {
    if (!statsEnabled) return FALSE;
    
    // Update display every few iterations
    static int updateCounter = 0;
    if (++updateCounter >= 100) { // Update every 100 packets
        char displayBuffer[1024];
        formatStatistics(displayBuffer, sizeof(displayBuffer));
        
        // Update UI text area
        IupSetAttribute(statsDisplay, "VALUE", displayBuffer);
        
        updateCounter = 0;
    }
    
    // Monitor packets (statistics are updated by other modules via updateStatistics)
    PacketNode *pac = head->next;
    while (pac != tail) {
        updateStatistics(pac, FALSE, FALSE); // Assume not dropped/modified by this module
        pac = pac->next;
    }
    
    return FALSE; // Don't modify packet flow
}

Module statsModule = {
    "Statistics",
    NAME,
    (short*)&statsEnabled,
    statsSetupUI,
    statsStartup,
    statsCloseDown,
    statsProcess,
    // runtime fields
    0, 0, NULL
};