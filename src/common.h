#pragma once
#include <string.h>
#include "iup.h"

#define CLUMPSY_VERSION "0.0"
#define MSG_BUFSIZE 256
#define NAME_SIZE 16

#define CONTROLS_HANDLE "__CONTROLS_HANDLE"
#define IS_YES(s) (!strcmp((s), "YES"));

// module
typedef struct {
    const char *name; // name of the module
    Ihandle* (*setupUIFunc)(); // return hbox as controls group
} Module;

extern Module dropModule;

// Iup GUI
void showStatus(const char* line);

// WinDivert
int divertStart(const char * filter, char buf[]);
void divertStop();
