#pragma once
#include "iup.h"

#define CLUMPSY_VERSION "0.0"
#define MSG_BUFSIZE 256
#define NAME_SIZE 16

// module
struct Module {
    const char *name; // name of the module
    Ihandle* (*setupUIFunc)(); // return hbox as controls group
};

extern Module dropModule;

// Iup GUI
void showStatus(const char* line);

// WinDivert
bool divertStart(const char * filter, char buf[]);
void divertStop();

