#pragma once

#include <windows.h>
#include "windivert.h"

#define APP_FILTER_TARGETS_BUFSIZE 512

typedef enum AppFilterMode
{
    APP_FILTER_MODE_PROCESS_NAME = 0,
    APP_FILTER_MODE_FULL_PATH = 1
} AppFilterMode;

typedef struct AppFilterConfig
{
    BOOL enabled;
    AppFilterMode mode;
    BOOL includeChildProcesses;
    char targets[APP_FILTER_TARGETS_BUFSIZE];
} AppFilterConfig;

typedef struct AppFilterStats
{
    LONG targetPidCount;
    LONG targetFlowCount;
    LONG targetEndpointCount;
    LONG affectedPackets;
    LONG passedUnmatchedPackets;
    LONG unknownPackets;
} AppFilterStats;

void AppFilterDefaultConfig(AppFilterConfig* config);
BOOL AppFilterStart(const AppFilterConfig* config, char* statusBuf, UINT statusBufLen);
void AppFilterStop(void);
BOOL AppFilterIsEnabled(void);
LONG AppFilterGetNetworkFilterVersion(void);
BOOL AppFilterBuildNetworkFilter(const char* baseFilter, char* filterBuf,
                                 UINT filterBufLen, BOOL* hasTargetFilters);
BOOL AppFilterShouldAffectPacket(const char* packet, UINT packetLen,
                                 const WINDIVERT_ADDRESS* addr);
void AppFilterGetStats(AppFilterStats* stats);
