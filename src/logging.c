// enhanced logging and data export module
#include <stdlib.h>
#include <Windows.h>
#include <time.h>
#include "iup.h"
#include "common.h"
#define NAME "logging"

// Log level definitions
typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN = 1,
    LOG_LEVEL_INFO = 2,
    LOG_LEVEL_DEBUG = 3,
    LOG_LEVEL_TRACE = 4
} LogLevel;

// Log format definitions
typedef enum {
    LOG_FORMAT_TEXT = 0,
    LOG_FORMAT_CSV = 1,
    LOG_FORMAT_JSON = 2,
    LOG_FORMAT_PCAP = 3
} LogFormat;

// Packet log entry structure
typedef struct {
    DWORD timestamp;
    UINT32 packetId;
    BOOL outbound;
    UINT16 protocol;
    UINT32 srcIP;
    UINT32 dstIP;
    UINT16 srcPort;
    UINT16 dstPort;
    UINT32 packetLen;
    char action[16]; // "PASS", "DROP", "LAG", "MODIFY"
    char module[16]; // which module processed it
    double latency;  // processing latency in ms
} PacketLogEntry;

static Ihandle *enabledCheckbox, *logLevelList, *logFormatList;
static Ihandle *logFileInput, *browseButton, *maxSizeInput;
static Ihandle *realTimeCheckbox, *autoExportCheckbox;
static Ihandle *exportButton, *clearLogButton;

static volatile short loggingEnabled = 0;
static volatile short logLevel = LOG_LEVEL_INFO;
static volatile short logFormat = LOG_FORMAT_TEXT;
static volatile short realTimeLogging = 1;
static volatile short autoExport = 0;
static volatile short maxLogSizeMB = 100;

static char logFilePath[512] = "clumsy_network.log";
static FILE* logFile = NULL;
static CRITICAL_SECTION logMutex;
static UINT32 packetCounter = 0;
static UINT64 totalLogSize = 0;

// Helper function to get timestamp string
static void getTimestamp(char* buffer, size_t bufferSize) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    snprintf(buffer, bufferSize, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

// Helper function to get protocol name
static const char* getProtocolName(UINT16 protocol) {
    switch (protocol) {
        case 1: return "ICMP";
        case 6: return "TCP";
        case 17: return "UDP";
        case 47: return "GRE";
        case 50: return "ESP";
        case 51: return "AH";
        default: return "OTHER";
    }
}

// Log packet entry in specified format
static void logPacketEntry(PacketLogEntry* entry) {
    if (!logFile || !loggingEnabled) return;
    
    EnterCriticalSection(&logMutex);
    
    char timestamp[32];
    getTimestamp(timestamp, sizeof(timestamp));
    
    switch (logFormat) {
        case LOG_FORMAT_TEXT:
            fprintf(logFile, "[%s] ID:%u %s %s %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u (%s) len:%u action:%s module:%s latency:%.2fms\n",
                timestamp, entry->packetId,
                entry->outbound ? "OUT" : "IN",
                getProtocolName(entry->protocol),
                (entry->srcIP >> 24) & 0xFF, (entry->srcIP >> 16) & 0xFF, (entry->srcIP >> 8) & 0xFF, entry->srcIP & 0xFF,
                entry->srcPort,
                (entry->dstIP >> 24) & 0xFF, (entry->dstIP >> 16) & 0xFF, (entry->dstIP >> 8) & 0xFF, entry->dstIP & 0xFF,
                entry->dstPort,
                getProtocolName(entry->protocol),
                entry->packetLen, entry->action, entry->module, entry->latency);
            break;
            
        case LOG_FORMAT_CSV:
            fprintf(logFile, "%s,%u,%s,%s,%u.%u.%u.%u,%u,%u.%u.%u.%u,%u,%u,%s,%s,%.2f\n",
                timestamp, entry->packetId,
                entry->outbound ? "OUT" : "IN",
                getProtocolName(entry->protocol),
                (entry->srcIP >> 24) & 0xFF, (entry->srcIP >> 16) & 0xFF, (entry->srcIP >> 8) & 0xFF, entry->srcIP & 0xFF,
                entry->srcPort,
                (entry->dstIP >> 24) & 0xFF, (entry->dstIP >> 16) & 0xFF, (entry->dstIP >> 8) & 0xFF, entry->dstIP & 0xFF,
                entry->dstPort, entry->packetLen, entry->action, entry->module, entry->latency);
            break;
            
        case LOG_FORMAT_JSON:
            fprintf(logFile, "{\"timestamp\":\"%s\",\"id\":%u,\"direction\":\"%s\",\"protocol\":\"%s\","
                "\"src_ip\":\"%u.%u.%u.%u\",\"src_port\":%u,\"dst_ip\":\"%u.%u.%u.%u\",\"dst_port\":%u,"
                "\"length\":%u,\"action\":\"%s\",\"module\":\"%s\",\"latency\":%.2f}\n",
                timestamp, entry->packetId,
                entry->outbound ? "outbound" : "inbound",
                getProtocolName(entry->protocol),
                (entry->srcIP >> 24) & 0xFF, (entry->srcIP >> 16) & 0xFF, (entry->srcIP >> 8) & 0xFF, entry->srcIP & 0xFF,
                entry->srcPort,
                (entry->dstIP >> 24) & 0xFF, (entry->dstIP >> 16) & 0xFF, (entry->dstIP >> 8) & 0xFF, entry->dstIP & 0xFF,
                entry->dstPort, entry->packetLen, entry->action, entry->module, entry->latency);
            break;
    }
    
    fflush(logFile);
    totalLogSize = ftell(logFile);
    
    // Check if log file exceeds maximum size
    if (totalLogSize > (maxLogSizeMB * 1024 * 1024)) {
        if (autoExport) {
            // Create archived log file with timestamp
            char archiveName[512];
            snprintf(archiveName, sizeof(archiveName), "%s.%s.archive", logFilePath, timestamp);
            fclose(logFile);
            MoveFile(logFilePath, archiveName);
            
            // Create new log file
            logFile = fopen(logFilePath, "w");
            if (logFile && logFormat == LOG_FORMAT_CSV) {
                fprintf(logFile, "timestamp,id,direction,protocol,src_ip,src_port,dst_ip,dst_port,length,action,module,latency\n");
            }
        }
    }
    
    LeaveCriticalSection(&logMutex);
}

// Log packet from processing pipeline
static void logPacketFromNode(PacketNode* pac, const char* action, const char* module) {
    if (logLevel < LOG_LEVEL_INFO) return;
    
    PacketLogEntry entry = {0};
    entry.timestamp = GetTickCount();
    entry.packetId = ++packetCounter;
    entry.outbound = pac->addr.Outbound;
    entry.packetLen = pac->packetLen;
    strcpy(entry.action, action);
    strcpy(entry.module, module);
    
    // Parse packet headers
    PWINDIVERT_IPHDR ipHeader = NULL;
    PWINDIVERT_TCPHDR tcpHeader = NULL;
    PWINDIVERT_UDPHDR udpHeader = NULL;
    
    WinDivertHelperParsePacket(pac->packet, pac->packetLen, &ipHeader, NULL, NULL, NULL, NULL,
                              &tcpHeader, &udpHeader, NULL, NULL, NULL, NULL);
    
    if (ipHeader) {
        entry.protocol = ipHeader->Protocol;
        entry.srcIP = ntohl(ipHeader->SrcAddr);
        entry.dstIP = ntohl(ipHeader->DstAddr);
        
        if (tcpHeader) {
            entry.srcPort = ntohs(tcpHeader->SrcPort);
            entry.dstPort = ntohs(tcpHeader->DstPort);
        } else if (udpHeader) {
            entry.srcPort = ntohs(udpHeader->SrcPort);
            entry.dstPort = ntohs(udpHeader->DstPort);
        }
    }
    
    logPacketEntry(&entry);
}

// Browse button callback
static int browseButtonCallback(Ihandle *ih) {
    UNREFERENCED_PARAMETER(ih);
    
    Ihandle *filedlg = IupFileDlg();
    IupSetAttribute(filedlg, "DIALOGTYPE", "SAVE");
    IupSetAttribute(filedlg, "TITLE", "Select Log File");
    IupSetAttribute(filedlg, "FILTER", "*.log");
    IupSetAttribute(filedlg, "FILTERINFO", "Log files");
    
    IupPopup(filedlg, IUP_CENTER, IUP_CENTER);
    
    if (IupGetInt(filedlg, "STATUS") != -1) {
        char* filename = IupGetAttribute(filedlg, "VALUE");
        IupStoreAttribute(logFileInput, "VALUE", filename);
        strcpy(logFilePath, filename);
    }
    
    IupDestroy(filedlg);
    return IUP_DEFAULT;
}

// Export button callback
static int exportButtonCallback(Ihandle *ih) {
    UNREFERENCED_PARAMETER(ih);
    
    if (logFile) {
        fflush(logFile);
        LOG("Log file exported/flushed: %s (%.2f MB)", logFilePath, totalLogSize / (1024.0 * 1024.0));
    }
    return IUP_DEFAULT;
}

// Clear log button callback
static int clearLogCallback(Ihandle *ih) {
    UNREFERENCED_PARAMETER(ih);
    
    EnterCriticalSection(&logMutex);
    if (logFile) {
        fclose(logFile);
        logFile = fopen(logFilePath, "w");
        if (logFile && logFormat == LOG_FORMAT_CSV) {
            fprintf(logFile, "timestamp,id,direction,protocol,src_ip,src_port,dst_ip,dst_port,length,action,module,latency\n");
        }
        totalLogSize = 0;
        packetCounter = 0;
    }
    LeaveCriticalSection(&logMutex);
    
    LOG("Log file cleared");
    return IUP_DEFAULT;
}

// Log level change callback
static int logLevelCallback(Ihandle *ih, char *text, int item, int state) {
    UNREFERENCED_PARAMETER(ih);
    UNREFERENCED_PARAMETER(text);
    if (state == 1) {
        logLevel = item - 1; // Convert from 1-based to 0-based
        LOG("Log level changed to: %d", logLevel);
    }
    return IUP_DEFAULT;
}

// Log format change callback
static int logFormatCallback(Ihandle *ih, char *text, int item, int state) {
    UNREFERENCED_PARAMETER(ih);
    UNREFERENCED_PARAMETER(text);
    if (state == 1) {
        logFormat = item - 1; // Convert from 1-based to 0-based
        LOG("Log format changed to: %d", logFormat);
        
        // Restart log file with new format
        if (logFile) {
            fclose(logFile);
            logFile = fopen(logFilePath, "w");
            if (logFile && logFormat == LOG_FORMAT_CSV) {
                fprintf(logFile, "timestamp,id,direction,protocol,src_ip,src_port,dst_ip,dst_port,length,action,module,latency\n");
            }
        }
    }
    return IUP_DEFAULT;
}

static Ihandle* loggingSetupUI() {
    Ihandle *loggingControlsBox = IupVbox(
        IupHbox(
            enabledCheckbox = IupToggle("Enable Logging", NULL),
            IupLabel("Level:"),
            logLevelList = IupList(NULL),
            IupLabel("Format:"),
            logFormatList = IupList(NULL),
            NULL
        ),
        IupHbox(
            IupLabel("Log File:"),
            logFileInput = IupText(NULL),
            browseButton = IupButton("Browse", NULL),
            NULL
        ),
        IupHbox(
            IupLabel("Max Size(MB):"),
            maxSizeInput = IupText(NULL),
            realTimeCheckbox = IupToggle("Real-time", NULL),
            autoExportCheckbox = IupToggle("Auto-export", NULL),
            NULL
        ),
        IupHbox(
            exportButton = IupButton("Export Now", NULL),
            clearLogButton = IupButton("Clear Log", NULL),
            NULL
        ),
        NULL
    );

    // Setup log level list
    IupSetAttribute(logLevelList, "DROPDOWN", "YES");
    IupSetAttribute(logLevelList, "VISIBLECOLUMNS", "8");
    IupSetAttribute(logLevelList, "1", "Error");
    IupSetAttribute(logLevelList, "2", "Warning");
    IupSetAttribute(logLevelList, "3", "Info");
    IupSetAttribute(logLevelList, "4", "Debug");
    IupSetAttribute(logLevelList, "5", "Trace");
    IupSetAttribute(logLevelList, "VALUE", "3"); // Default to Info
    IupSetCallback(logLevelList, "ACTION", (Icallback)logLevelCallback);

    // Setup log format list
    IupSetAttribute(logFormatList, "DROPDOWN", "YES");
    IupSetAttribute(logFormatList, "VISIBLECOLUMNS", "8");
    IupSetAttribute(logFormatList, "1", "Text");
    IupSetAttribute(logFormatList, "2", "CSV");
    IupSetAttribute(logFormatList, "3", "JSON");
    IupSetAttribute(logFormatList, "4", "PCAP");
    IupSetAttribute(logFormatList, "VALUE", "1"); // Default to Text
    IupSetCallback(logFormatList, "ACTION", (Icallback)logFormatCallback);

    // Setup log file input
    IupSetAttribute(logFileInput, "EXPAND", "HORIZONTAL");
    IupSetAttribute(logFileInput, "VALUE", logFilePath);

    // Setup browse button
    IupSetCallback(browseButton, "ACTION", (Icallback)browseButtonCallback);
    IupSetAttribute(browseButton, "PADDING", "4x");

    // Setup max size input
    IupSetAttribute(maxSizeInput, "VISIBLECOLUMNS", "6");
    IupSetAttribute(maxSizeInput, "VALUE", "100");
    IupSetCallback(maxSizeInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(maxSizeInput, SYNCED_VALUE, (char*)&maxLogSizeMB);
    IupSetAttribute(maxSizeInput, INTEGER_MIN, "1");
    IupSetAttribute(maxSizeInput, INTEGER_MAX, "10000");

    // Setup checkboxes
    IupSetCallback(enabledCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(enabledCheckbox, SYNCED_VALUE, (char*)&loggingEnabled);

    IupSetCallback(realTimeCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(realTimeCheckbox, SYNCED_VALUE, (char*)&realTimeLogging);
    IupSetAttribute(realTimeCheckbox, "VALUE", "ON");

    IupSetCallback(autoExportCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(autoExportCheckbox, SYNCED_VALUE, (char*)&autoExport);

    // Setup buttons
    IupSetCallback(exportButton, "ACTION", (Icallback)exportButtonCallback);
    IupSetAttribute(exportButton, "PADDING", "4x");

    IupSetCallback(clearLogButton, "ACTION", (Icallback)clearLogCallback);
    IupSetAttribute(clearLogButton, "PADDING", "4x");

    if (parameterized) {
        setFromParameter(enabledCheckbox, "VALUE", NAME"-enabled");
        setFromParameter(logLevelList, "VALUE", NAME"-level");
        setFromParameter(logFormatList, "VALUE", NAME"-format");
        setFromParameter(logFileInput, "VALUE", NAME"-file");
        setFromParameter(maxSizeInput, "VALUE", NAME"-maxsize");
        setFromParameter(realTimeCheckbox, "VALUE", NAME"-realtime");
        setFromParameter(autoExportCheckbox, "VALUE", NAME"-autoexport");
    }

    return loggingControlsBox;
}

static void loggingStartup() {
    InitializeCriticalSection(&logMutex);
    
    // Open log file
    logFile = fopen(logFilePath, "a");
    if (logFile) {
        if (logFormat == LOG_FORMAT_CSV && ftell(logFile) == 0) {
            fprintf(logFile, "timestamp,id,direction,protocol,src_ip,src_port,dst_ip,dst_port,length,action,module,latency\n");
        }
        totalLogSize = ftell(logFile);
    }
    
    LOG("Enhanced logging enabled: level=%d, format=%d, file=%s", logLevel, logFormat, logFilePath);
}

static void loggingCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
    
    if (logFile) {
        fclose(logFile);
        logFile = NULL;
    }
    
    DeleteCriticalSection(&logMutex);
    LOG("Enhanced logging disabled");
}

static short loggingProcess(PacketNode *head, PacketNode* tail) {
    if (!loggingEnabled || !realTimeLogging) return FALSE;
    
    // Log all packets passing through
    PacketNode *pac = head->next;
    while (pac != tail) {
        logPacketFromNode(pac, "PASS", "logging");
        pac = pac->next;
    }
    
    return FALSE; // Don't modify packet flow
}

Module loggingModule = {
    "Enhanced Logging",
    NAME,
    (short*)&loggingEnabled,
    loggingSetupUI,
    loggingStartup,
    loggingCloseDown,
    loggingProcess,
    // runtime fields
    0, 0, NULL
};