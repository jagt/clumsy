#pragma once
#include <string.h>
#include "iup.h"
#include "divert.h"

#define CLUMPSY_VERSION "0.0"
#define MSG_BUFSIZE 256
#define NAME_SIZE 16
#define MODULE_CNT 1

#define CONTROLS_HANDLE "__CONTROLS_HANDLE"
#define SYNCED_VALUE "__SYNCED_VALUE"
#define IS_YES(s) (!strcmp((s), "YES"));

// package node
typedef struct _NODE {
    char *packet;
    UINT packetLen;
    DIVERT_ADDRESS addr;
    struct _NODE *prev, *next;
} PacketNode;

PacketNode* createNode(char* buf, UINT len, DIVERT_ADDRESS *addr);
void freeNode(PacketNode *node);
PacketNode* popNode(PacketNode *node);
PacketNode* insertAfter(PacketNode *node, PacketNode *target);
PacketNode* appendNode(PacketNode *node);

// module
typedef struct {
    /*
     * Static module data
     */
    const char *name; // name of the module
    short *enabledFlag; // volatile short flag to determine enabled or not
    Ihandle* (*setupUIFunc)(); // return hbox as controls group
    void (*startUp)(); // called when starting up the module
    void (*closeDown)(); // called when starting up the module
    void (*process)(PacketNode *head, PacketNode *tail);
    /*
     * Flags used during program excution. Need to be re initialized on each run
     */
    short lastEnabled; // if it is enabled on last run
} Module;

extern Module dropModule;
extern Module* modules[MODULE_CNT]; // all modules in a list

// Iup GUI
void showStatus(const char* line);

// WinDivert
int divertStart(const char * filter, char buf[]);
void divertStop();

