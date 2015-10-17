#pragma once
#include <stdio.h>
#include <assert.h>
#include "iup.h"
#include "windivert.h"

#define CLUMSY_VERSION "0.3"
#define MSG_BUFSIZE 512
#define FILTER_BUFSIZE 1024
#define NAME_SIZE 16
#define MODULE_CNT 7
#define ICON_UPDATE_MS 200

#define CONTROLS_HANDLE "__CONTROLS_HANDLE"
#define SYNCED_VALUE "__SYNCED_VALUE"
#define INTEGER_MAX "__INTEGER_MAX"
#define INTEGER_MIN "__INTEGER_MIN"
#define FIXED_MAX "__FIXED_MAX"
#define FIXED_MIN "__FIXED_MIN"
#define FIXED_EPSILON 0.01

// workaround stupid vs2012 runtime check.
// it would show even when seeing explicit "(short)(i);"
#define I2S(x) ((short)((x) & 0xFFFF))


#ifdef __MINGW32__
#define INLINE_FUNCTION __inline__
#else
#define INLINE_FUNCTION __inline
#endif


// my mingw seems missing some of the functions
// undef all mingw linked interlock* and use __atomic gcc builtins
#ifdef __MINGW32__
// and 16 seems to be broken
#ifdef InterlockedAnd16
#undef InterlockedAnd16
#endif
#define InterlockedAnd16(p, val) (__atomic_and_fetch((short*)(p), (val), __ATOMIC_SEQ_CST))

#ifdef InterlockedExchange16
#undef InterlockedExchange16
#endif
#define InterlockedExchange16(p, val) (__atomic_exchange_n((short*)(p), (val), __ATOMIC_SEQ_CST))

#ifdef InterlockedIncrement16
#undef InterlockedIncrement16
#endif
#define InterlockedIncrement16(p) (__atomic_add_fetch((short*)(p), 1, __ATOMIC_SEQ_CST))

#ifdef InterlockedDecrement16
#undef InterlockedDecrement16
#endif
#define InterlockedDecrement16(p) (__atomic_sub_fetch((short*)(p), 1, __ATOMIC_SEQ_CST))

#endif



#ifdef _DEBUG
#define ABORT() assert(0)
#ifdef __MINGW32__
#define LOG(fmt, ...) (printf("%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__))
#else
#define LOG(fmt, ...) (printf(__FUNCTION__ ": " fmt "\n", ##__VA_ARGS__))
#endif

// check for assert
#ifndef assert
// some how vs can't trigger debugger on assert, which is really stupid
#define assert(x) do {if (!(x)) {DebugBreak();} } while(0)
#endif


#else
#define LOG(fmt, ...)
#define ABORT()
//#define assert(x)
#endif

// package node
typedef struct _NODE {
    char *packet;
    UINT packetLen;
    WINDIVERT_ADDRESS addr;
    DWORD timestamp; // ! timestamp isn't filled when creating node since it's only needed for lag
    struct _NODE *prev, *next;
} PacketNode;

void initPacketNodeList();
PacketNode* createNode(char* buf, UINT len, WINDIVERT_ADDRESS *addr);
void freeNode(PacketNode *node);
PacketNode* popNode(PacketNode *node);
PacketNode* insertBefore(PacketNode *node, PacketNode *target);
PacketNode* insertAfter(PacketNode *node, PacketNode *target);
PacketNode* appendNode(PacketNode *node);
short isListEmpty();

// shared ui handlers
int uiSyncChance(Ihandle *ih);
int uiSyncToggle(Ihandle *ih, int state);
int uiSyncInteger(Ihandle *ih);
int uiSyncFixed(Ihandle *ih);


// module
typedef struct {
    /*
     * Static module data
     */
    const char *displayName; // display name shown in ui
    const char *shortName; // single word name
    short *enabledFlag; // volatile short flag to determine enabled or not
    Ihandle* (*setupUIFunc)(); // return hbox as controls group
    void (*startUp)(); // called when starting up the module
    void (*closeDown)(PacketNode *head, PacketNode *tail); // called when starting up the module
    short (*process)(PacketNode *head, PacketNode *tail);
    /*
     * Flags used during program excution. Need to be re initialized on each run
     */
    short lastEnabled; // if it is enabled on last run
    short processTriggered; // whether this module has been triggered in last step 
    Ihandle *iconHandle; // store the icon to be updated
} Module;

extern Module lagModule;
extern Module dropModule;
extern Module throttleModule;
extern Module oodModule;
extern Module dupModule;
extern Module tamperModule;
extern Module resetModule;
extern Module capModule;
extern Module* modules[MODULE_CNT]; // all modules in a list

// status for sending packets, 
#define SEND_STATUS_NONE 0
#define SEND_STATUS_SEND 1
#define SEND_STATUS_FAIL -1
extern volatile short sendState;


// Iup GUI
void showStatus(const char* line);

// WinDivert
int divertStart(const char * filter, char buf[]);
void divertStop();

// utils
// STR to convert int macro to string
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

short calcChance(short chance);

#define BOUND_TEXT(b) ((b) == WINDIVERT_DIRECTION_INBOUND ? "IN" : "OUT")
#define IS_INBOUND(b) ((b) == WINDIVERT_DIRECTION_INBOUND)
#define IS_OUTBOUND(b) ((b) == WINDIVERT_DIRECTION_OUTBOUND)
// inline helper for inbound outbound check
static INLINE_FUNCTION
BOOL checkDirection(UINT8 dir, short inbound, short outbound) {
    return (inbound && IS_INBOUND(dir))
                || (outbound && IS_OUTBOUND(dir));
}



// wraped timeBegin/EndPeriod to keep calling safe and end when exit
#define TIMER_RESOLUTION 4
void startTimePeriod();
void endTimePeriod();

// elevate
BOOL IsElevated();
BOOL IsRunAsAdmin();
BOOL tryElevate(HWND hWnd, BOOL silent);

// icons
extern const unsigned char icon8x8[8*8];

// parameterized
extern BOOL parameterized;
void setFromParameter(Ihandle *ih, const char *field, const char *key);
BOOL parseArgs(int argc, char* argv[]);

