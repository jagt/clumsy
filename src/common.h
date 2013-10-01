#pragma once

#define CLUMPSY_VERSION "0.0"
#define MSG_BUFSIZE 256

// Iup GUI
void showStatus(const char* line);

// WinDivert
bool divertStart(const char * filter, char buf[]);
void divertStop();