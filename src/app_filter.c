#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tlhelp32.h>
#include <iphlpapi.h>
#include <tcpmib.h>
#include <udpmib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "app_filter.h"
#include "common.h"

#define APP_FILTER_MAX_TARGETS     32
#define APP_FILTER_TARGET_LEN      APP_FILTER_TARGETS_BUFSIZE
#define APP_FILTER_MAX_FLOWS       16384
#define APP_FILTER_MAX_ENDPOINTS   8192
#define APP_FILTER_MAX_PID_CACHE   2048
#define APP_FILTER_MAX_TARGET_PORTS 256
#define APP_FILTER_PID_CACHE_MS    3000
#define APP_FILTER_REFRESH_MS      1000
#define APP_FILTER_FLOW_LINGER_MS  15000
#define APP_FILTER_PORT_LINGER_MS  30000
#define APP_FILTER_FLOW_PRIORITY   0

#pragma pack(push, 1)
typedef struct FlowKey
{
    UINT8 protocol;
    UINT8 localAddr[16];
    UINT8 remoteAddr[16];
    UINT16 localPort;
    UINT16 remotePort;
} FlowKey;
#pragma pack(pop)

typedef struct EndpointKey
{
    UINT8 protocol;
    UINT8 localAddr[16];
    UINT16 localPort;
} EndpointKey;

typedef struct FlowEntry
{
    BOOL active;
    BOOL seeded;
    FlowKey key;
    UINT64 endpointId;
    DWORD pid;
    BOOL isTarget;
    BOOL lingering;
    DWORD lastSeenTick;
} FlowEntry;

typedef struct EndpointEntry
{
    BOOL active;
    EndpointKey key;
    DWORD pid;
    BOOL isTarget;
    DWORD lastSeenTick;
} EndpointEntry;

typedef struct PidCacheEntry
{
    BOOL active;
    DWORD pid;
    BOOL isTarget;
    DWORD lastCheckedTick;
} PidCacheEntry;

typedef struct TargetPortEntry
{
    BOOL active;
    UINT8 protocol;
    UINT16 localPort;
    DWORD lastSeenTick;
} TargetPortEntry;

typedef struct TargetEntry
{
    char text[APP_FILTER_TARGET_LEN];
    BOOL hasExeSuffix;
} TargetEntry;

static CRITICAL_SECTION appFilterLock;
static BOOL lockInitialized = FALSE;
static volatile LONG appFilterEnabled = FALSE;
static volatile LONG stopFlowThread = FALSE;
static volatile LONG refreshInProgress = FALSE;
static HANDLE flowHandle = INVALID_HANDLE_VALUE;
static HANDLE flowThread = NULL;

static AppFilterMode configuredMode = APP_FILTER_MODE_PROCESS_NAME;
static BOOL includeChildProcesses = TRUE;
static TargetEntry targets[APP_FILTER_MAX_TARGETS];
static UINT targetCount = 0;
static FlowEntry flows[APP_FILTER_MAX_FLOWS];
static EndpointEntry endpoints[APP_FILTER_MAX_ENDPOINTS];
static PidCacheEntry pidCache[APP_FILTER_MAX_PID_CACHE];
static TargetPortEntry targetPorts[APP_FILTER_MAX_TARGET_PORTS];
static AppFilterStats stats;
static DWORD lastRefreshTick = 0;
static volatile LONG networkFilterVersion = 0;

static DWORD WINAPI AppFilterFlowThread(LPVOID arg);
static BOOL ParseTargets(const char* targetText, AppFilterMode mode,
                         char* statusBuf, UINT statusBufLen);
static void SeedIpHelperTables(BOOL clearSeededFirst);
static void BumpNetworkFilterVersion(void);

static void CopyString(char* dst, UINT dstLen, const char* src)
{
    UINT ix;

    if (dstLen == 0)
    {
        return;
    }
    if (src == NULL)
    {
        dst[0] = '\0';
        return;
    }
    for (ix = 0; ix + 1 < dstLen && src[ix] != '\0'; ++ix)
    {
        dst[ix] = src[ix];
    }
    dst[ix] = '\0';
}

static void SetStatus(char* statusBuf, UINT statusBufLen, const char* status)
{
    CopyString(statusBuf, statusBufLen, status);
}

static BOOL IsBlank(const char* text)
{
    const unsigned char* p = (const unsigned char*)text;

    if (p == NULL)
    {
        return TRUE;
    }
    while (*p != '\0')
    {
        if (!isspace(*p))
        {
            return FALSE;
        }
        ++p;
    }
    return TRUE;
}

static void NormalizePathText(char* text)
{
    unsigned char* p = (unsigned char*)text;

    while (*p != '\0')
    {
        if (*p == '/')
        {
            *p = '\\';
        }
        else
        {
            *p = (unsigned char)tolower(*p);
        }
        ++p;
    }
}

static const char* BaseName(const char* path)
{
    const char* slash;
    const char* backslash;

    if (path == NULL)
    {
        return "";
    }
    slash = strrchr(path, '/');
    backslash = strrchr(path, '\\');
    if (slash == NULL || (backslash != NULL && backslash > slash))
    {
        slash = backslash;
    }
    return slash == NULL ? path : slash + 1;
}

static BOOL EndsWithExe(const char* text)
{
    size_t len;

    if (text == NULL)
    {
        return FALSE;
    }
    len = strlen(text);
    if (len < 4)
    {
        return FALSE;
    }
    return text[len - 4] == '.' &&
        text[len - 3] == 'e' &&
        text[len - 2] == 'x' &&
        text[len - 1] == 'e';
}

static BOOL SameText(const char* left, const char* right)
{
    return strcmp(left, right) == 0;
}

static void BumpNetworkFilterVersion(void)
{
    InterlockedIncrement(&networkFilterVersion);
}

static BOOL MatchNormalizedProcessName(const char* processName)
{
    UINT ix;
    char processNoExe[APP_FILTER_TARGET_LEN];

    for (ix = 0; ix < targetCount; ++ix)
    {
        if (SameText(processName, targets[ix].text))
        {
            return TRUE;
        }
        if (!targets[ix].hasExeSuffix && EndsWithExe(processName))
        {
            CopyString(processNoExe, sizeof(processNoExe), processName);
            processNoExe[strlen(processNoExe) - 4] = '\0';
            if (SameText(processNoExe, targets[ix].text))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

static BOOL MatchNormalizedFullPath(const char* path)
{
    UINT ix;

    for (ix = 0; ix < targetCount; ++ix)
    {
        if (SameText(path, targets[ix].text))
        {
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL MatchProcessPath(const char* path)
{
    char normalized[APP_FILTER_TARGET_LEN];
    const char* name;

    if (path == NULL || path[0] == '\0')
    {
        return FALSE;
    }
    CopyString(normalized, sizeof(normalized), path);
    NormalizePathText(normalized);
    if (configuredMode == APP_FILTER_MODE_FULL_PATH)
    {
        return MatchNormalizedFullPath(normalized);
    }
    name = BaseName(normalized);
    return MatchNormalizedProcessName(name);
}

static BOOL SnapshotProcessInfo(DWORD pid, DWORD* parentPid, char* exeName,
                                UINT exeNameLen)
{
    HANDLE snapshot;
    PROCESSENTRY32 entry;
    BOOL found = FALSE;

    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    memset(&entry, 0, sizeof(entry));
    entry.dwSize = sizeof(entry);
    if (Process32First(snapshot, &entry))
    {
        do
        {
            if (entry.th32ProcessID == pid)
            {
                if (parentPid != NULL)
                {
                    *parentPid = entry.th32ParentProcessID;
                }
                if (exeName != NULL && exeNameLen > 0)
                {
                    CopyString(exeName, exeNameLen, entry.szExeFile);
                }
                found = TRUE;
                break;
            }
        }
        while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return found;
}

static BOOL SnapshotMatchPid(DWORD pid)
{
    char exeName[APP_FILTER_TARGET_LEN];
    char normalized[APP_FILTER_TARGET_LEN];

    if (!SnapshotProcessInfo(pid, NULL, exeName, sizeof(exeName)))
    {
        return FALSE;
    }

    CopyString(normalized, sizeof(normalized), exeName);
    NormalizePathText(normalized);
    return MatchNormalizedProcessName(BaseName(normalized));
}

static BOOL QueryPidDirectMatchesTarget(DWORD pid, char* detailBuf, UINT detailBufLen)
{
    HANDLE process;
    DWORD pathLen;
    char path[APP_FILTER_TARGET_LEN];
    BOOL matched = FALSE;

    if (pid == 0)
    {
        return FALSE;
    }

    process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (process != NULL)
    {
        pathLen = sizeof(path);
        if (QueryFullProcessImageNameA(process, 0, path, &pathLen))
        {
            path[sizeof(path) - 1] = '\0';
            CopyString(detailBuf, detailBufLen, path);
            matched = MatchProcessPath(path);
        }
        CloseHandle(process);
    }

    if (!matched && configuredMode == APP_FILTER_MODE_PROCESS_NAME)
    {
        matched = SnapshotMatchPid(pid);
        if (matched && (detailBuf == NULL || detailBuf[0] == '\0'))
        {
            CopyString(detailBuf, detailBufLen, "snapshot process name");
        }
    }

    return matched;
}

static BOOL QueryPidChildMatchesTarget(DWORD pid, DWORD* matchedParentPid)
{
    DWORD currentPid;
    DWORD parentPid;
    UINT depth;

    currentPid = pid;
    for (depth = 0; depth < 32; ++depth)
    {
        if (!SnapshotProcessInfo(currentPid, &parentPid, NULL, 0))
        {
            return FALSE;
        }
        if (parentPid == 0 || parentPid == currentPid)
        {
            return FALSE;
        }
        if (QueryPidDirectMatchesTarget(parentPid, NULL, 0))
        {
            if (matchedParentPid != NULL)
            {
                *matchedParentPid = parentPid;
            }
            return TRUE;
        }
        currentPid = parentPid;
    }

    return FALSE;
}

static BOOL QueryPidMatchesTarget(DWORD pid)
{
    char detail[APP_FILTER_TARGET_LEN];
    DWORD matchedParentPid = 0;
    BOOL matched;

    detail[0] = '\0';
    matched = QueryPidDirectMatchesTarget(pid, detail, sizeof(detail));
    if (!matched && includeChildProcesses)
    {
        matched = QueryPidChildMatchesTarget(pid, &matchedParentPid);
    }

    if (matchedParentPid != 0)
    {
        LOG("App filter PID %lu is target (via parent %lu)", pid, matchedParentPid);
    }
    else if (matched)
    {
        LOG("App filter PID %lu is target (%s)", pid, detail[0] ? detail : "no path");
    }
    return matched;
}

static void RecountPidTargetsLocked(void)
{
    UINT ix;
    LONG count = 0;

    for (ix = 0; ix < APP_FILTER_MAX_PID_CACHE; ++ix)
    {
        if (pidCache[ix].active && pidCache[ix].isTarget)
        {
            ++count;
        }
    }
    InterlockedExchange(&stats.targetPidCount, count);
}

static BOOL IsTargetPid(DWORD pid)
{
    UINT ix;
    UINT freeIx = APP_FILTER_MAX_PID_CACHE;
    UINT oldestIx = 0;
    DWORD now = GetTickCount();
    DWORD oldestTick = now;
    BOOL foundOldest = FALSE;
    BOOL matched;

    if (pid == 0)
    {
        return FALSE;
    }

    EnterCriticalSection(&appFilterLock);
    for (ix = 0; ix < APP_FILTER_MAX_PID_CACHE; ++ix)
    {
        if (pidCache[ix].active && pidCache[ix].pid == pid)
        {
            if (now - pidCache[ix].lastCheckedTick <= APP_FILTER_PID_CACHE_MS)
            {
                matched = pidCache[ix].isTarget;
                LeaveCriticalSection(&appFilterLock);
                return matched;
            }
            freeIx = ix;
            break;
        }
        if (!pidCache[ix].active && freeIx == APP_FILTER_MAX_PID_CACHE)
        {
            freeIx = ix;
        }
        if (pidCache[ix].active &&
            (!foundOldest || pidCache[ix].lastCheckedTick - oldestTick > 0x80000000UL))
        {
            oldestIx = ix;
            oldestTick = pidCache[ix].lastCheckedTick;
            foundOldest = TRUE;
        }
    }
    LeaveCriticalSection(&appFilterLock);

    matched = QueryPidMatchesTarget(pid);

    EnterCriticalSection(&appFilterLock);
    if (freeIx == APP_FILTER_MAX_PID_CACHE)
    {
        freeIx = foundOldest ? oldestIx : 0;
    }
    pidCache[freeIx].active = TRUE;
    pidCache[freeIx].pid = pid;
    pidCache[freeIx].isTarget = matched;
    pidCache[freeIx].lastCheckedTick = now;
    RecountPidTargetsLocked();
    LeaveCriticalSection(&appFilterLock);

    return matched;
}

static int CompareFlowKey(const FlowKey* left, const FlowKey* right)
{
    return memcmp(left, right, sizeof(*left));
}

static int CompareEndpointKey(const EndpointKey* left, const EndpointKey* right)
{
    return memcmp(left, right, sizeof(*left));
}

static BOOL IsPortFilterable(UINT8 protocol, UINT16 localPort)
{
    return localPort != 0 && (protocol == IPPROTO_TCP || protocol == IPPROTO_UDP);
}

static void SetIpv4MappedAddress(UINT8 addr[16], UINT32 ipv4HostOrder)
{
    UINT32 words[4];

    words[0] = 0;
    words[1] = 0;
    words[2] = 0x0000FFFF;
    words[3] = ipv4HostOrder;
    memcpy(addr, words, sizeof(words));
}

static void SetZeroAddress(UINT8 addr[16])
{
    memset(addr, 0, 16);
}

static void SetHostIpv6Address(UINT8 addr[16], const UINT32 words[4])
{
    memcpy(addr, words, 16);
}

static void SetNetworkIpv6Address(UINT8 addr[16], const UINT8 networkBytes[16])
{
    UINT32 netWords[4];
    UINT32 hostWords[4];

    memcpy(netWords, networkBytes, sizeof(netWords));
    WinDivertHelperNtohIPv6Address(netWords, hostWords);
    SetHostIpv6Address(addr, hostWords);
}

static void FlowKeyFromFlowAddress(FlowKey* key, const WINDIVERT_ADDRESS* addr)
{
    memset(key, 0, sizeof(*key));
    key->protocol = addr->Flow.Protocol;
    memcpy(key->localAddr, addr->Flow.LocalAddr, sizeof(key->localAddr));
    memcpy(key->remoteAddr, addr->Flow.RemoteAddr, sizeof(key->remoteAddr));
    key->localPort = addr->Flow.LocalPort;
    key->remotePort = addr->Flow.RemotePort;
}

static void ReverseFlowKey(FlowKey* reversed, const FlowKey* key)
{
    memset(reversed, 0, sizeof(*reversed));
    reversed->protocol = key->protocol;
    memcpy(reversed->localAddr, key->remoteAddr, sizeof(reversed->localAddr));
    memcpy(reversed->remoteAddr, key->localAddr, sizeof(reversed->remoteAddr));
    reversed->localPort = key->remotePort;
    reversed->remotePort = key->localPort;
}

static void EndpointKeyFromFlowKey(EndpointKey* endpoint, const FlowKey* key)
{
    memset(endpoint, 0, sizeof(*endpoint));
    endpoint->protocol = key->protocol;
    memcpy(endpoint->localAddr, key->localAddr, sizeof(endpoint->localAddr));
    endpoint->localPort = key->localPort;
}

static int FindFlowByKeyLocked(const FlowKey* key)
{
    UINT ix;

    for (ix = 0; ix < APP_FILTER_MAX_FLOWS; ++ix)
    {
        if (flows[ix].active && CompareFlowKey(&flows[ix].key, key) == 0)
        {
            return (int)ix;
        }
    }
    return -1;
}

static int FindFlowByEndpointLocked(UINT64 endpointId)
{
    UINT ix;

    if (endpointId == 0)
    {
        return -1;
    }
    for (ix = 0; ix < APP_FILTER_MAX_FLOWS; ++ix)
    {
        if (flows[ix].active && flows[ix].endpointId == endpointId)
        {
            return (int)ix;
        }
    }
    return -1;
}

static int AllocateFlowSlotLocked(void)
{
    UINT ix;
    UINT oldestIx = 0;
    DWORD oldestTick = 0;
    BOOL foundOldest = FALSE;

    for (ix = 0; ix < APP_FILTER_MAX_FLOWS; ++ix)
    {
        if (!flows[ix].active)
        {
            return (int)ix;
        }
        if (!foundOldest || flows[ix].lastSeenTick - oldestTick > 0x80000000UL)
        {
            oldestIx = ix;
            oldestTick = flows[ix].lastSeenTick;
            foundOldest = TRUE;
        }
    }
    return (int)oldestIx;
}

static int FindTargetPortLocked(UINT8 protocol, UINT16 localPort)
{
    UINT ix;

    for (ix = 0; ix < APP_FILTER_MAX_TARGET_PORTS; ++ix)
    {
        if (targetPorts[ix].active &&
            targetPorts[ix].protocol == protocol &&
            targetPorts[ix].localPort == localPort)
        {
            return (int)ix;
        }
    }
    return -1;
}

static BOOL IsTargetPortRememberedLocked(UINT8 protocol, UINT16 localPort)
{
    return IsPortFilterable(protocol, localPort) &&
        FindTargetPortLocked(protocol, localPort) >= 0;
}

static int AllocateTargetPortSlotLocked(void)
{
    UINT ix;
    UINT oldestIx = 0;
    DWORD oldestTick = 0;
    BOOL foundOldest = FALSE;

    for (ix = 0; ix < APP_FILTER_MAX_TARGET_PORTS; ++ix)
    {
        if (!targetPorts[ix].active)
        {
            return (int)ix;
        }
        if (!foundOldest ||
            targetPorts[ix].lastSeenTick - oldestTick > 0x80000000UL)
        {
            oldestIx = ix;
            oldestTick = targetPorts[ix].lastSeenTick;
            foundOldest = TRUE;
        }
    }
    return (int)oldestIx;
}

static BOOL RememberTargetPortLocked(UINT8 protocol, UINT16 localPort)
{
    int slot;
    BOOL wasActive;
    BOOL changed;

    if (!IsPortFilterable(protocol, localPort))
    {
        return FALSE;
    }

    slot = FindTargetPortLocked(protocol, localPort);
    if (slot < 0)
    {
        slot = AllocateTargetPortSlotLocked();
    }

    wasActive = targetPorts[slot].active;
    changed = !wasActive ||
        targetPorts[slot].protocol != protocol ||
        targetPorts[slot].localPort != localPort;
    targetPorts[slot].active = TRUE;
    targetPorts[slot].protocol = protocol;
    targetPorts[slot].localPort = localPort;
    targetPorts[slot].lastSeenTick = GetTickCount();
    return changed;
}

static BOOL PurgeExpiredTargetPortsLocked(DWORD now)
{
    UINT ix;
    BOOL changed = FALSE;

    for (ix = 0; ix < APP_FILTER_MAX_TARGET_PORTS; ++ix)
    {
        if (targetPorts[ix].active &&
            now - targetPorts[ix].lastSeenTick >= APP_FILTER_PORT_LINGER_MS)
        {
            memset(&targetPorts[ix], 0, sizeof(targetPorts[ix]));
            changed = TRUE;
        }
    }
    return changed;
}

static void AddOrUpdateFlow(const FlowKey* key, UINT64 endpointId, DWORD pid,
                            BOOL isTarget, BOOL seeded)
{
    int slot;
    BOOL wasActive;
    BOOL wasTarget;
    BOOL newSeeded;

    EnterCriticalSection(&appFilterLock);
    if (!isTarget && IsTargetPortRememberedLocked(key->protocol, key->localPort))
    {
        isTarget = TRUE;
    }
    slot = FindFlowByKeyLocked(key);
    if (slot < 0)
    {
        slot = AllocateFlowSlotLocked();
    }

    wasActive = flows[slot].active;
    wasTarget = flows[slot].isTarget;
    newSeeded = wasActive ? (flows[slot].seeded && seeded) : seeded;

    flows[slot].active = TRUE;
    flows[slot].seeded = newSeeded;
    flows[slot].key = *key;
    flows[slot].endpointId = endpointId;
    flows[slot].pid = pid;
    flows[slot].isTarget = isTarget;
    flows[slot].lingering = FALSE;
    flows[slot].lastSeenTick = GetTickCount();

    if (wasActive && wasTarget && !isTarget)
    {
        InterlockedDecrement(&stats.targetFlowCount);
    }
    else if ((!wasActive || !wasTarget) && isTarget)
    {
        InterlockedIncrement(&stats.targetFlowCount);
    }
    if (isTarget && RememberTargetPortLocked(key->protocol, key->localPort))
    {
        BumpNetworkFilterVersion();
    }
    LeaveCriticalSection(&appFilterLock);
}

static void RemoveFlowByEndpointOrKey(const FlowKey* key, UINT64 endpointId)
{
    int slot;
    DWORD now = GetTickCount();

    EnterCriticalSection(&appFilterLock);
    slot = key != NULL ? FindFlowByKeyLocked(key) : -1;
    if (slot < 0)
    {
        slot = FindFlowByEndpointLocked(endpointId);
    }
    if (slot >= 0)
    {
        if (flows[slot].isTarget)
        {
            flows[slot].lingering = TRUE;
            flows[slot].endpointId = 0;
            flows[slot].lastSeenTick = now;
        }
        else
        {
            memset(&flows[slot], 0, sizeof(flows[slot]));
        }
    }
    LeaveCriticalSection(&appFilterLock);
}

static int FindEndpointByKeyLocked(const EndpointKey* key)
{
    UINT ix;

    for (ix = 0; ix < APP_FILTER_MAX_ENDPOINTS; ++ix)
    {
        if (endpoints[ix].active && CompareEndpointKey(&endpoints[ix].key, key) == 0)
        {
            return (int)ix;
        }
    }
    return -1;
}

static int AllocateEndpointSlotLocked(void)
{
    UINT ix;
    UINT oldestIx = 0;
    DWORD oldestTick = 0;
    BOOL foundOldest = FALSE;

    for (ix = 0; ix < APP_FILTER_MAX_ENDPOINTS; ++ix)
    {
        if (!endpoints[ix].active)
        {
            return (int)ix;
        }
        if (!foundOldest || endpoints[ix].lastSeenTick - oldestTick > 0x80000000UL)
        {
            oldestIx = ix;
            oldestTick = endpoints[ix].lastSeenTick;
            foundOldest = TRUE;
        }
    }
    return (int)oldestIx;
}

static void AddOrUpdateEndpoint(const EndpointKey* key, DWORD pid, BOOL isTarget)
{
    int slot;
    BOOL wasActive;
    BOOL wasTarget;

    EnterCriticalSection(&appFilterLock);
    if (!isTarget && IsTargetPortRememberedLocked(key->protocol, key->localPort))
    {
        isTarget = TRUE;
    }
    slot = FindEndpointByKeyLocked(key);
    if (slot < 0)
    {
        slot = AllocateEndpointSlotLocked();
    }

    wasActive = endpoints[slot].active;
    wasTarget = endpoints[slot].isTarget;

    endpoints[slot].active = TRUE;
    endpoints[slot].key = *key;
    endpoints[slot].pid = pid;
    endpoints[slot].isTarget = isTarget;
    endpoints[slot].lastSeenTick = GetTickCount();

    if (wasActive && wasTarget && !isTarget)
    {
        InterlockedDecrement(&stats.targetEndpointCount);
    }
    else if ((!wasActive || !wasTarget) && isTarget)
    {
        InterlockedIncrement(&stats.targetEndpointCount);
    }
    if (isTarget && RememberTargetPortLocked(key->protocol, key->localPort))
    {
        BumpNetworkFilterVersion();
    }
    LeaveCriticalSection(&appFilterLock);
}

static void ClearSeededEntries(void)
{
    UINT ix;

    EnterCriticalSection(&appFilterLock);
    for (ix = 0; ix < APP_FILTER_MAX_FLOWS; ++ix)
    {
        if (flows[ix].active && flows[ix].seeded)
        {
            if (flows[ix].isTarget)
            {
                InterlockedDecrement(&stats.targetFlowCount);
            }
            memset(&flows[ix], 0, sizeof(flows[ix]));
        }
    }
    for (ix = 0; ix < APP_FILTER_MAX_ENDPOINTS; ++ix)
    {
        if (endpoints[ix].active)
        {
            if (endpoints[ix].isTarget)
            {
                InterlockedDecrement(&stats.targetEndpointCount);
            }
            memset(&endpoints[ix], 0, sizeof(endpoints[ix]));
        }
    }
    LeaveCriticalSection(&appFilterLock);
}

static void PurgeExpiredLingeringFlowsLocked(DWORD now)
{
    UINT ix;

    for (ix = 0; ix < APP_FILTER_MAX_FLOWS; ++ix)
    {
        if (flows[ix].active && flows[ix].isTarget && flows[ix].lingering &&
            now - flows[ix].lastSeenTick >= APP_FILTER_FLOW_LINGER_MS)
        {
            InterlockedDecrement(&stats.targetFlowCount);
            memset(&flows[ix], 0, sizeof(flows[ix]));
        }
    }
}

static BOOL LookupFlowLocked(const FlowKey* key, BOOL* isTarget)
{
    int slot;

    slot = FindFlowByKeyLocked(key);
    if (slot < 0)
    {
        return FALSE;
    }
    flows[slot].lastSeenTick = GetTickCount();
    *isTarget = flows[slot].isTarget;
    return TRUE;
}

static BOOL LookupEndpointKeyLocked(const EndpointKey* key, BOOL* isTarget)
{
    int slot;

    slot = FindEndpointByKeyLocked(key);
    if (slot < 0)
    {
        return FALSE;
    }
    endpoints[slot].lastSeenTick = GetTickCount();
    *isTarget = endpoints[slot].isTarget;
    return TRUE;
}

static BOOL LookupUdpEndpointLocked(const FlowKey* key, BOOL* isTarget)
{
    EndpointKey endpoint;

    EndpointKeyFromFlowKey(&endpoint, key);
    if (LookupEndpointKeyLocked(&endpoint, isTarget))
    {
        return TRUE;
    }

    SetIpv4MappedAddress(endpoint.localAddr, 0);
    if (LookupEndpointKeyLocked(&endpoint, isTarget))
    {
        return TRUE;
    }

    SetZeroAddress(endpoint.localAddr);
    return LookupEndpointKeyLocked(&endpoint, isTarget);
}

static BOOL LookupLocalPortTargetLocked(const FlowKey* key, BOOL* isTarget)
{
    UINT ix;

    if (!IsPortFilterable(key->protocol, key->localPort))
    {
        return FALSE;
    }

    for (ix = 0; ix < APP_FILTER_MAX_TARGET_PORTS; ++ix)
    {
        if (targetPorts[ix].active &&
            targetPorts[ix].protocol == key->protocol &&
            targetPorts[ix].localPort == key->localPort)
        {
            targetPorts[ix].lastSeenTick = GetTickCount();
            *isTarget = TRUE;
            return TRUE;
        }
    }

    for (ix = 0; ix < APP_FILTER_MAX_FLOWS; ++ix)
    {
        if (flows[ix].active &&
            flows[ix].isTarget &&
            flows[ix].key.protocol == key->protocol &&
            flows[ix].key.localPort == key->localPort)
        {
            flows[ix].lastSeenTick = GetTickCount();
            *isTarget = TRUE;
            return TRUE;
        }
    }

    for (ix = 0; ix < APP_FILTER_MAX_ENDPOINTS; ++ix)
    {
        if (endpoints[ix].active &&
            endpoints[ix].isTarget &&
            endpoints[ix].key.protocol == key->protocol &&
            endpoints[ix].key.localPort == key->localPort)
        {
            endpoints[ix].lastSeenTick = GetTickCount();
            *isTarget = TRUE;
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL BuildPacketFlowKey(const char* packet, UINT packetLen,
                               const WINDIVERT_ADDRESS* addr, FlowKey* key)
{
    PWINDIVERT_IPHDR ipHeader;
    PWINDIVERT_IPV6HDR ipv6Header;
    PWINDIVERT_TCPHDR tcpHeader;
    PWINDIVERT_UDPHDR udpHeader;
    UINT32 srcAddr6[4];
    UINT32 dstAddr6[4];
    UINT8 srcAddr[16];
    UINT8 dstAddr[16];
    UINT16 srcPort;
    UINT16 dstPort;

    ipHeader = NULL;
    ipv6Header = NULL;
    tcpHeader = NULL;
    udpHeader = NULL;
    memset(key, 0, sizeof(*key));

    if (addr == NULL ||
        !WinDivertHelperParsePacket(packet, packetLen, &ipHeader, &ipv6Header, NULL,
                                    NULL, NULL, &tcpHeader, &udpHeader, NULL, NULL, NULL, NULL))
    {
        return FALSE;
    }

    if (tcpHeader != NULL)
    {
        key->protocol = IPPROTO_TCP;
        srcPort = WinDivertHelperNtohs(tcpHeader->SrcPort);
        dstPort = WinDivertHelperNtohs(tcpHeader->DstPort);
    }
    else if (udpHeader != NULL)
    {
        key->protocol = IPPROTO_UDP;
        srcPort = WinDivertHelperNtohs(udpHeader->SrcPort);
        dstPort = WinDivertHelperNtohs(udpHeader->DstPort);
    }
    else
    {
        return FALSE;
    }

    if (ipHeader != NULL)
    {
        SetIpv4MappedAddress(srcAddr, WinDivertHelperNtohl(ipHeader->SrcAddr));
        SetIpv4MappedAddress(dstAddr, WinDivertHelperNtohl(ipHeader->DstAddr));
    }
    else if (ipv6Header != NULL)
    {
        WinDivertHelperNtohIPv6Address(ipv6Header->SrcAddr, srcAddr6);
        WinDivertHelperNtohIPv6Address(ipv6Header->DstAddr, dstAddr6);
        SetHostIpv6Address(srcAddr, srcAddr6);
        SetHostIpv6Address(dstAddr, dstAddr6);
    }
    else
    {
        return FALSE;
    }

    if (addr->Outbound)
    {
        memcpy(key->localAddr, srcAddr, sizeof(key->localAddr));
        memcpy(key->remoteAddr, dstAddr, sizeof(key->remoteAddr));
        key->localPort = srcPort;
        key->remotePort = dstPort;
    }
    else
    {
        memcpy(key->localAddr, dstAddr, sizeof(key->localAddr));
        memcpy(key->remoteAddr, srcAddr, sizeof(key->remoteAddr));
        key->localPort = dstPort;
        key->remotePort = srcPort;
    }

    return TRUE;
}

static BOOL LookupPacketTarget(const FlowKey* key, const WINDIVERT_ADDRESS* addr,
                               BOOL* isTarget)
{
    FlowKey reversed;

    EnterCriticalSection(&appFilterLock);
    if (LookupLocalPortTargetLocked(key, isTarget))
    {
        LeaveCriticalSection(&appFilterLock);
        return TRUE;
    }

    if (LookupFlowLocked(key, isTarget))
    {
        LeaveCriticalSection(&appFilterLock);
        return TRUE;
    }

    if (addr != NULL && addr->Loopback)
    {
        ReverseFlowKey(&reversed, key);
        if (LookupFlowLocked(&reversed, isTarget))
        {
            LeaveCriticalSection(&appFilterLock);
            return TRUE;
        }
    }

    if (key->protocol == IPPROTO_UDP)
    {
        if (LookupUdpEndpointLocked(key, isTarget))
        {
            LeaveCriticalSection(&appFilterLock);
            return TRUE;
        }
        if (addr != NULL && addr->Loopback)
        {
            ReverseFlowKey(&reversed, key);
            if (LookupUdpEndpointLocked(&reversed, isTarget))
            {
                LeaveCriticalSection(&appFilterLock);
                return TRUE;
            }
        }
    }

    LeaveCriticalSection(&appFilterLock);
    return FALSE;
}

static void MaybeRefreshSeededTables(void)
{
    DWORD now;

    if (!AppFilterIsEnabled())
    {
        return;
    }
    now = GetTickCount();
    if (now - lastRefreshTick < APP_FILTER_REFRESH_MS)
    {
        return;
    }
    if (InterlockedCompareExchange(&refreshInProgress, TRUE, FALSE) != FALSE)
    {
        return;
    }
    if (now - lastRefreshTick >= APP_FILTER_REFRESH_MS)
    {
        SeedIpHelperTables(TRUE);
        lastRefreshTick = GetTickCount();
    }
    InterlockedExchange(&refreshInProgress, FALSE);
}

static BOOL ParseTargets(const char* targetText, AppFilterMode mode,
                         char* statusBuf, UINT statusBufLen)
{
    char buf[APP_FILTER_TARGETS_BUFSIZE];
    char token[APP_FILTER_TARGET_LEN];
    char* start;
    char* end;
    char* p;
    UINT len;

    targetCount = 0;
    memset(targets, 0, sizeof(targets));

    if (IsBlank(targetText))
    {
        SetStatus(statusBuf, statusBufLen,
                  "Application filter target is empty. Enter a process name or path.");
        return FALSE;
    }

    CopyString(buf, sizeof(buf), targetText);
    start = buf;
    p = buf;
    for (;;)
    {
        if (*p == ',' || *p == ';' || *p == '\0')
        {
            end = p;
            while (start < end && isspace((unsigned char)*start))
            {
                ++start;
            }
            while (end > start && isspace((unsigned char)*(end - 1)))
            {
                --end;
            }
            len = (UINT)(end - start);
            if (len > 0)
            {
                if (targetCount >= APP_FILTER_MAX_TARGETS)
                {
                    SetStatus(statusBuf, statusBufLen,
                              "Application filter has too many targets.");
                    return FALSE;
                }
                if (len >= sizeof(token))
                {
                    len = sizeof(token) - 1;
                }
                memcpy(token, start, len);
                token[len] = '\0';
                NormalizePathText(token);
                if (mode == APP_FILTER_MODE_PROCESS_NAME)
                {
                    CopyString(targets[targetCount].text,
                               sizeof(targets[targetCount].text), BaseName(token));
                }
                else
                {
                    CopyString(targets[targetCount].text,
                               sizeof(targets[targetCount].text), token);
                }
                targets[targetCount].hasExeSuffix = EndsWithExe(targets[targetCount].text);
                ++targetCount;
            }
            if (*p == '\0')
            {
                break;
            }
            start = p + 1;
        }
        ++p;
    }

    if (targetCount == 0)
    {
        SetStatus(statusBuf, statusBufLen,
                  "Application filter target is empty. Enter a process name or path.");
        return FALSE;
    }
    return TRUE;
}

static void SeedTcp4Table(void)
{
    PMIB_TCPTABLE_OWNER_PID table;
    DWORD size;
    DWORD err;
    DWORD ix;
    FlowKey key;
    PMIB_TCPROW_OWNER_PID row;
    BOOL isTarget;

    size = 0;
    err = GetExtendedTcpTable(NULL, &size, FALSE, AF_INET,
                              TCP_TABLE_OWNER_PID_ALL, 0);
    if (err != ERROR_INSUFFICIENT_BUFFER)
    {
        LOG("GetExtendedTcpTable IPv4 sizing failed (%lu)", err);
        return;
    }

    table = (PMIB_TCPTABLE_OWNER_PID)malloc(size);
    if (table == NULL)
    {
        LOG("Failed to allocate IPv4 TCP table (%lu bytes)", size);
        return;
    }

    err = GetExtendedTcpTable(table, &size, FALSE, AF_INET,
                              TCP_TABLE_OWNER_PID_ALL, 0);
    if (err == NO_ERROR)
    {
        for (ix = 0; ix < table->dwNumEntries; ++ix)
        {
            row = &table->table[ix];
            memset(&key, 0, sizeof(key));
            key.protocol = IPPROTO_TCP;
            SetIpv4MappedAddress(key.localAddr, WinDivertHelperNtohl(row->dwLocalAddr));
            SetIpv4MappedAddress(key.remoteAddr, WinDivertHelperNtohl(row->dwRemoteAddr));
            key.localPort = WinDivertHelperNtohs((UINT16)row->dwLocalPort);
            key.remotePort = WinDivertHelperNtohs((UINT16)row->dwRemotePort);
            isTarget = IsTargetPid(row->dwOwningPid);
            AddOrUpdateFlow(&key, 0, row->dwOwningPid, isTarget, TRUE);
        }
    }
    else
    {
        LOG("GetExtendedTcpTable IPv4 failed (%lu)", err);
    }

    free(table);
}

static void SeedTcp6Table(void)
{
    PMIB_TCP6TABLE_OWNER_PID table;
    DWORD size;
    DWORD err;
    DWORD ix;
    FlowKey key;
    PMIB_TCP6ROW_OWNER_PID row;
    BOOL isTarget;

    size = 0;
    err = GetExtendedTcpTable(NULL, &size, FALSE, AF_INET6,
                              TCP_TABLE_OWNER_PID_ALL, 0);
    if (err != ERROR_INSUFFICIENT_BUFFER)
    {
        LOG("GetExtendedTcpTable IPv6 sizing failed (%lu)", err);
        return;
    }

    table = (PMIB_TCP6TABLE_OWNER_PID)malloc(size);
    if (table == NULL)
    {
        LOG("Failed to allocate IPv6 TCP table (%lu bytes)", size);
        return;
    }

    err = GetExtendedTcpTable(table, &size, FALSE, AF_INET6,
                              TCP_TABLE_OWNER_PID_ALL, 0);
    if (err == NO_ERROR)
    {
        for (ix = 0; ix < table->dwNumEntries; ++ix)
        {
            row = &table->table[ix];
            memset(&key, 0, sizeof(key));
            key.protocol = IPPROTO_TCP;
            SetNetworkIpv6Address(key.localAddr, row->ucLocalAddr);
            SetNetworkIpv6Address(key.remoteAddr, row->ucRemoteAddr);
            key.localPort = WinDivertHelperNtohs((UINT16)row->dwLocalPort);
            key.remotePort = WinDivertHelperNtohs((UINT16)row->dwRemotePort);
            isTarget = IsTargetPid(row->dwOwningPid);
            AddOrUpdateFlow(&key, 0, row->dwOwningPid, isTarget, TRUE);
        }
    }
    else
    {
        LOG("GetExtendedTcpTable IPv6 failed (%lu)", err);
    }

    free(table);
}

static void SeedUdp4Table(void)
{
    PMIB_UDPTABLE_OWNER_PID table;
    DWORD size;
    DWORD err;
    DWORD ix;
    EndpointKey key;
    PMIB_UDPROW_OWNER_PID row;
    BOOL isTarget;

    size = 0;
    err = GetExtendedUdpTable(NULL, &size, FALSE, AF_INET,
                              UDP_TABLE_OWNER_PID, 0);
    if (err != ERROR_INSUFFICIENT_BUFFER)
    {
        LOG("GetExtendedUdpTable IPv4 sizing failed (%lu)", err);
        return;
    }

    table = (PMIB_UDPTABLE_OWNER_PID)malloc(size);
    if (table == NULL)
    {
        LOG("Failed to allocate IPv4 UDP table (%lu bytes)", size);
        return;
    }

    err = GetExtendedUdpTable(table, &size, FALSE, AF_INET,
                              UDP_TABLE_OWNER_PID, 0);
    if (err == NO_ERROR)
    {
        for (ix = 0; ix < table->dwNumEntries; ++ix)
        {
            row = &table->table[ix];
            memset(&key, 0, sizeof(key));
            key.protocol = IPPROTO_UDP;
            SetIpv4MappedAddress(key.localAddr, WinDivertHelperNtohl(row->dwLocalAddr));
            key.localPort = WinDivertHelperNtohs((UINT16)row->dwLocalPort);
            isTarget = IsTargetPid(row->dwOwningPid);
            AddOrUpdateEndpoint(&key, row->dwOwningPid, isTarget);
        }
    }
    else
    {
        LOG("GetExtendedUdpTable IPv4 failed (%lu)", err);
    }

    free(table);
}

static void SeedUdp6Table(void)
{
    PMIB_UDP6TABLE_OWNER_PID table;
    DWORD size;
    DWORD err;
    DWORD ix;
    EndpointKey key;
    PMIB_UDP6ROW_OWNER_PID row;
    BOOL isTarget;

    size = 0;
    err = GetExtendedUdpTable(NULL, &size, FALSE, AF_INET6,
                              UDP_TABLE_OWNER_PID, 0);
    if (err != ERROR_INSUFFICIENT_BUFFER)
    {
        LOG("GetExtendedUdpTable IPv6 sizing failed (%lu)", err);
        return;
    }

    table = (PMIB_UDP6TABLE_OWNER_PID)malloc(size);
    if (table == NULL)
    {
        LOG("Failed to allocate IPv6 UDP table (%lu bytes)", size);
        return;
    }

    err = GetExtendedUdpTable(table, &size, FALSE, AF_INET6,
                              UDP_TABLE_OWNER_PID, 0);
    if (err == NO_ERROR)
    {
        for (ix = 0; ix < table->dwNumEntries; ++ix)
        {
            row = &table->table[ix];
            memset(&key, 0, sizeof(key));
            key.protocol = IPPROTO_UDP;
            SetNetworkIpv6Address(key.localAddr, row->ucLocalAddr);
            key.localPort = WinDivertHelperNtohs((UINT16)row->dwLocalPort);
            isTarget = IsTargetPid(row->dwOwningPid);
            AddOrUpdateEndpoint(&key, row->dwOwningPid, isTarget);
        }
    }
    else
    {
        LOG("GetExtendedUdpTable IPv6 failed (%lu)", err);
    }

    free(table);
}

static void SeedIpHelperTables(BOOL clearSeededFirst)
{
    if (!AppFilterIsEnabled())
    {
        return;
    }
    if (clearSeededFirst)
    {
        ClearSeededEntries();
    }
    SeedTcp4Table();
    SeedTcp6Table();
    SeedUdp4Table();
    SeedUdp6Table();
    LOG("App filter seeded tables: target pids=%ld flows=%ld endpoints=%ld",
        stats.targetPidCount, stats.targetFlowCount, stats.targetEndpointCount);
}

static BOOL AppendText(char* dst, UINT dstLen, const char* src)
{
    size_t used;
    size_t add;

    if (dstLen == 0 || src == NULL)
    {
        return FALSE;
    }
    used = strlen(dst);
    add = strlen(src);
    if (used + add + 1 > dstLen)
    {
        return FALSE;
    }
    memcpy(dst + used, src, add + 1);
    return TRUE;
}

static BOOL AppendFormat(char* dst, UINT dstLen, const char* fmt, ...)
{
    char buf[256];
    va_list args;
    int written;

    va_start(args, fmt);
    written = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (written < 0 || written >= (int)sizeof(buf))
    {
        return FALSE;
    }
    return AppendText(dst, dstLen, buf);
}

static BOOL AppendTargetPortClause(char* dst, UINT dstLen,
                                   const TargetPortEntry* port, BOOL* hasClause)
{
    const char* proto;

    if (!IsPortFilterable(port->protocol, port->localPort))
    {
        return TRUE;
    }

    proto = port->protocol == IPPROTO_TCP ? "tcp" : "udp";
    if (*hasClause && !AppendText(dst, dstLen, " or "))
    {
        return FALSE;
    }
    if (!AppendFormat(dst, dstLen,
                      "(%s and (%s.SrcPort == %u or %s.DstPort == %u))",
                      proto, proto, (UINT)port->localPort, proto, (UINT)port->localPort))
    {
        return FALSE;
    }
    *hasClause = TRUE;
    return TRUE;
}

LONG AppFilterGetNetworkFilterVersion(void)
{
    if (AppFilterIsEnabled() && lockInitialized)
    {
        EnterCriticalSection(&appFilterLock);
        if (PurgeExpiredTargetPortsLocked(GetTickCount()))
        {
            BumpNetworkFilterVersion();
        }
        PurgeExpiredLingeringFlowsLocked(GetTickCount());
        LeaveCriticalSection(&appFilterLock);
    }
    return InterlockedCompareExchange(&networkFilterVersion, 0, 0);
}

BOOL AppFilterBuildNetworkFilter(const char* baseFilter, char* filterBuf,
                                 UINT filterBufLen, BOOL* hasTargetFilters)
{
    char targetFilter[FILTER_BUFSIZE];
    const char* base;
    UINT ix;
    BOOL hasClause = FALSE;
    BOOL ok = TRUE;

    if (filterBuf == NULL || filterBufLen == 0)
    {
        return FALSE;
    }
    filterBuf[0] = '\0';
    if (hasTargetFilters != NULL)
    {
        *hasTargetFilters = FALSE;
    }

    base = IsBlank(baseFilter) ? "true" : baseFilter;
    targetFilter[0] = '\0';

    if (!AppFilterIsEnabled())
    {
        CopyString(filterBuf, filterBufLen, base);
        return TRUE;
    }

    EnterCriticalSection(&appFilterLock);
    if (PurgeExpiredTargetPortsLocked(GetTickCount()))
    {
        BumpNetworkFilterVersion();
    }
    PurgeExpiredLingeringFlowsLocked(GetTickCount());
    for (ix = 0; ok && ix < APP_FILTER_MAX_TARGET_PORTS; ++ix)
    {
        if (targetPorts[ix].active)
        {
            ok = AppendTargetPortClause(targetFilter, sizeof(targetFilter),
                                        &targetPorts[ix], &hasClause);
        }
    }
    LeaveCriticalSection(&appFilterLock);

    if (!ok)
    {
        LOG("Application target network filter exceeded %u bytes.",
            (UINT)sizeof(targetFilter));
        if (!hasClause)
        {
            CopyString(targetFilter, sizeof(targetFilter), "false");
        }
    }
    else if (!hasClause)
    {
        CopyString(targetFilter, sizeof(targetFilter), "false");
    }

    if (!AppendText(filterBuf, filterBufLen, "(") ||
        !AppendText(filterBuf, filterBufLen, base) ||
        !AppendText(filterBuf, filterBufLen, ") and (") ||
        !AppendText(filterBuf, filterBufLen, targetFilter) ||
        !AppendText(filterBuf, filterBufLen, ")"))
    {
        CopyString(filterBuf, filterBufLen, "false");
        hasClause = FALSE;
        LOG("Application network filter exceeded %u bytes; capture is paused.",
            filterBufLen);
    }

    if (hasTargetFilters != NULL)
    {
        *hasTargetFilters = hasClause;
    }
    return TRUE;
}

void AppFilterDefaultConfig(AppFilterConfig* config)
{
    if (config == NULL)
    {
        return;
    }
    memset(config, 0, sizeof(*config));
    config->enabled = FALSE;
    config->mode = APP_FILTER_MODE_PROCESS_NAME;
    config->includeChildProcesses = TRUE;
    config->targets[0] = '\0';
}

BOOL AppFilterStart(const AppFilterConfig* config, char* statusBuf, UINT statusBufLen)
{
    AppFilterConfig localConfig;

    if (config == NULL)
    {
        AppFilterDefaultConfig(&localConfig);
        config = &localConfig;
    }

    AppFilterStop();
    if (!config->enabled)
    {
        SetStatus(statusBuf, statusBufLen, "");
        return TRUE;
    }

    configuredMode = config->mode;
    includeChildProcesses = config->includeChildProcesses;
    if (!ParseTargets(config->targets, configuredMode, statusBuf, statusBufLen))
    {
        return FALSE;
    }
    memset(flows, 0, sizeof(flows));
    memset(endpoints, 0, sizeof(endpoints));
    memset(pidCache, 0, sizeof(pidCache));
    memset(targetPorts, 0, sizeof(targetPorts));
    memset(&stats, 0, sizeof(stats));
    InterlockedExchange(&networkFilterVersion, 1);
    InitializeCriticalSection(&appFilterLock);
    lockInitialized = TRUE;
    InterlockedExchange(&stopFlowThread, FALSE);
    InterlockedExchange(&refreshInProgress, FALSE);
    InterlockedExchange(&appFilterEnabled, TRUE);

    lastRefreshTick = GetTickCount();
    SeedIpHelperTables(FALSE);

    flowHandle = WinDivertOpen("true", WINDIVERT_LAYER_FLOW, APP_FILTER_FLOW_PRIORITY,
                               WINDIVERT_FLAG_SNIFF | WINDIVERT_FLAG_RECV_ONLY);
    if (flowHandle == INVALID_HANDLE_VALUE)
    {
        sprintf(statusBuf, "Failed to start application filter flow tracker (code:%lu).",
                GetLastError());
        AppFilterStop();
        return FALSE;
    }

    flowThread = CreateThread(NULL, 1, AppFilterFlowThread, NULL, 0, NULL);
    if (flowThread == NULL)
    {
        sprintf(statusBuf, "Failed to create application filter flow thread (%lu).",
                GetLastError());
        AppFilterStop();
        return FALSE;
    }

    LOG("Application filter started with %u target(s).", targetCount);
    SetStatus(statusBuf, statusBufLen, "Application filter started.");
    return TRUE;
}

void AppFilterStop(void)
{
    HANDLE thread;
    HANDLE handle;

    InterlockedExchange(&appFilterEnabled, FALSE);
    InterlockedExchange(&stopFlowThread, TRUE);

    thread = flowThread;
    handle = flowHandle;
    if (handle != INVALID_HANDLE_VALUE)
    {
        WinDivertShutdown(handle, WINDIVERT_SHUTDOWN_RECV);
    }
    if (thread != NULL)
    {
        WaitForSingleObject(thread, INFINITE);
        CloseHandle(thread);
        flowThread = NULL;
    }
    if (handle != INVALID_HANDLE_VALUE)
    {
        WinDivertClose(handle);
        flowHandle = INVALID_HANDLE_VALUE;
    }

    if (lockInitialized)
    {
        EnterCriticalSection(&appFilterLock);
        memset(flows, 0, sizeof(flows));
        memset(endpoints, 0, sizeof(endpoints));
        memset(pidCache, 0, sizeof(pidCache));
        memset(targetPorts, 0, sizeof(targetPorts));
        memset(&stats, 0, sizeof(stats));
        targetCount = 0;
        LeaveCriticalSection(&appFilterLock);
        DeleteCriticalSection(&appFilterLock);
        lockInitialized = FALSE;
    }
    includeChildProcesses = TRUE;
    InterlockedExchange(&networkFilterVersion, 0);
}

BOOL AppFilterIsEnabled(void)
{
    return InterlockedCompareExchange(&appFilterEnabled, FALSE, FALSE) != FALSE;
}

BOOL AppFilterShouldAffectPacket(const char* packet, UINT packetLen,
                                 const WINDIVERT_ADDRESS* addr)
{
    FlowKey key;
    BOOL isTarget;

    if (!AppFilterIsEnabled())
    {
        return TRUE;
    }

    if (!BuildPacketFlowKey(packet, packetLen, addr, &key))
    {
        InterlockedIncrement(&stats.unknownPackets);
        return FALSE;
    }

    if (LookupPacketTarget(&key, addr, &isTarget))
    {
        if (isTarget)
        {
            InterlockedIncrement(&stats.affectedPackets);
            return TRUE;
        }
        InterlockedIncrement(&stats.passedUnmatchedPackets);
        return FALSE;
    }

    InterlockedIncrement(&stats.unknownPackets);
    MaybeRefreshSeededTables();
    return FALSE;
}

void AppFilterGetStats(AppFilterStats* outStats)
{
    if (outStats == NULL)
    {
        return;
    }
    *outStats = stats;
}

static DWORD WINAPI AppFilterFlowThread(LPVOID arg)
{
    WINDIVERT_ADDRESS addr;
    FlowKey key;
    BOOL isTarget;
    DWORD lastError;

    UNREFERENCED_PARAMETER(arg);

    for (;;)
    {
        memset(&addr, 0, sizeof(addr));
        if (!WinDivertRecv(flowHandle, NULL, 0, NULL, &addr))
        {
            lastError = GetLastError();
            if (InterlockedCompareExchange(&stopFlowThread, FALSE, FALSE) ||
                lastError == ERROR_OPERATION_ABORTED ||
                lastError == ERROR_INVALID_HANDLE)
            {
                return 0;
            }
            LOG("Application filter flow recv failed (%lu)", lastError);
            Sleep(10);
            continue;
        }

        if (!AppFilterIsEnabled())
        {
            continue;
        }

        if (addr.Event == WINDIVERT_EVENT_FLOW_ESTABLISHED)
        {
            FlowKeyFromFlowAddress(&key, &addr);
            isTarget = IsTargetPid(addr.Flow.ProcessId);
            AddOrUpdateFlow(&key, addr.Flow.EndpointId, addr.Flow.ProcessId,
                            isTarget, FALSE);
            if (isTarget)
            {
                LOG("Application filter tracked target flow pid=%lu endpoint=%I64u",
                    addr.Flow.ProcessId, addr.Flow.EndpointId);
            }
        }
        else if (addr.Event == WINDIVERT_EVENT_FLOW_DELETED)
        {
            FlowKeyFromFlowAddress(&key, &addr);
            RemoveFlowByEndpointOrKey(&key, addr.Flow.EndpointId);
        }
    }
}
