#pragma once
#include <stdio.h>
#include <assert.h>
#include "iup.h"
#include "divert.h"

#define CLUMSY_VERSION "0.0"
#define MSG_BUFSIZE 512
#define NAME_SIZE 16
#define MODULE_CNT 4
#define ICON_UPDATE_MS 200

#define CONTROLS_HANDLE "__CONTROLS_HANDLE"
#define SYNCED_VALUE "__SYNCED_VALUE"
#define INTEGER_MAX "__INTEGER_MAX"
#define INTEGER_MIN "__INTEGER_MIN"


#ifdef __MINGW32__
#define INLINE_FUNCTION __inline__
#else
#define INLINE_FUNCTION __inline
#endif


// seems mingw has no InterlockedExchange16, use builtin __atomic instead
// TODO but seems 64bit have this, need to figure out why
#ifdef __MINGW32__
#ifndef InterlockedExchange16
#define InterlockedExchange16(p, val) (__atomic_exchange_n((short*)(p), (val), __ATOMIC_SEQ_CST))
#endif
#ifndef InterlockedIncrement16
#define InterlockedIncrement16(p) (__atomic_add_fetch((short*)(p), 1, __ATOMIC_SEQ_CST))
#endif
#endif

#ifdef _DEBUG
#ifdef __MINGW32__
#define LOG(fmt, ...) (printf("%s: " fmt "\n", __FUNCTION__, ##__VA_ARGS__))
#else
#define LOG(fmt, ...) (printf(__FUNCTION__ ": " fmt "\n", ##__VA_ARGS__))
#endif
// some how vs can't trigger debugger on assert, which is really stupid
//#define assert(x) do {if (!(x)) {DebugBreak();} } while(0)
#else
#define LOG(fmt, ...)
//#define assert(x)
#endif

// package node
typedef struct _NODE {
    char *packet;
    UINT packetLen;
    DIVERT_ADDRESS addr;
    DWORD timestamp; // ! timestamp isn't filled when creating node since it's only needed for lag
    struct _NODE *prev, *next;
} PacketNode;

void initPacketNodeList();
PacketNode* createNode(char* buf, UINT len, DIVERT_ADDRESS *addr);
void freeNode(PacketNode *node);
PacketNode* popNode(PacketNode *node);
PacketNode* insertAfter(PacketNode *node, PacketNode *target);
PacketNode* appendNode(PacketNode *node);
short isListEmpty();

// shared ui handlers
int uiSyncChance(Ihandle *ih);
int uiSyncToggle(Ihandle *ih, int state);
int uiSyncInteger(Ihandle *ih);

// module
typedef struct {
    /*
     * Static module data
     */
    const char *name; // name of the module
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
extern Module* modules[MODULE_CNT]; // all modules in a list

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

#define BOUND_TEXT(b) ((b) == DIVERT_DIRECTION_INBOUND ? "IN" : "OUT")
#define IS_INBOUND(b) ((b) == DIVERT_DIRECTION_INBOUND)
#define IS_OUTBOUND(b) ((b) == DIVERT_DIRECTION_OUTBOUND)
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
BOOL tryElevate(HWND hWnd);

// icons
extern const unsigned char icon8x8[8*8];