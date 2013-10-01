#include <iostream>
#include "iup.h"

#include "common.h"
using namespace std;

// global iup handlers
Ihandle *dialog, *topFrame, *bottomFrame; 
Ihandle *statusLabel;
Ihandle *filterText, *filterButton;

void showStatus(const char *line) {
    IupStoreAttribute(statusLabel, "TITLE", line); 
}

static int uiStopCb(Ihandle *ih);
static int uiStartCb(Ihandle *ih) {
    char buf[MSG_BUFSIZE];
    if (divertStart(IupGetAttribute(filterText, "VALUE"), buf) == false) {
        showStatus(buf);
        return IUP_DEFAULT;
    }

    // successfully started
    showStatus("Started filtering. Enable functionalities to take effect.");
    IupSetAttribute(filterText, "ACTIVE", "NO");
    IupSetAttribute(filterButton, "TITLE", "Stop");
    IupSetCallback(filterButton, "ACTION", uiStopCb);
    return IUP_DEFAULT;
}

static int uiStopCb(Ihandle *ih) {
    showStatus("Input filtering criteria and click start.");
    IupSetAttribute(filterText, "ACTIVE", "YES");
    IupSetAttribute(filterButton, "TITLE", "Start");
    IupSetCallback(filterButton, "ACTION", uiStartCb);
    return IUP_DEFAULT;
}


void init(int argc, char* argv[]) {
    IupOpen(&argc, &argv);

    // setup controls
    Ihandle *topVbox, *dialogVBox;

    statusLabel = IupLabel("Input filtering criteria and click start.");
    IupSetAttribute(statusLabel, "EXPAND", "HORIZONTAL");
    IupSetAttribute(statusLabel, "ALIGNMENT", "ACENTER");

    topFrame = IupFrame(
        topVbox = IupVbox(
            filterText = IupText(NULL),
            filterButton = IupButton("Start", NULL)
        )
    );

    IupSetAttribute(topFrame, "TITLE", "Filtering");
    IupSetAttribute(topFrame, "EXPAND", "HORIZONTAL");
    IupSetAttribute(filterText, "EXPAND", "HORIZONTAL");
    IupSetCallback(filterButton, "ACTION", uiStartCb);


    dialog = IupDialog(
        dialogVBox = IupVbox(
            statusLabel,
            topFrame
        )
    );

    IupSetAttribute(dialog, "TITLE", "clumpsy " CLUMPSY_VERSION);
    IupSetAttribute(dialog, "SIZE", "360x400");
    IupSetAttribute(dialog, "RESIZE", "NO");

    // global layout settings to affect childrens
    IupSetAttribute(dialogVBox, "ALIGNMENT", "ACENTER");
    IupSetAttribute(dialogVBox, "CMARGIN", "4x4");
    IupSetAttribute(dialogVBox, "CGAP", "4x2");
    IupSetAttribute(dialogVBox, "PADDING", "6x2");

    // kickoff event loops
    IupShowXY(dialog, IUP_CENTER, IUP_CENTER);

}

void startup() {
    IupMainLoop();
}

void cleanup() {
    IupClose();
}

int main(int argc, char* argv[]) {
    init(argc, argv);
    startup();
    cleanup();
    return 0;
}