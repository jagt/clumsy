#include <iostream>
#include "iup.h"

#include "common.h"
using namespace std;

// global iup handlers
static Ihandle *dialog, *topFrame, *bottomFrame; 
static Ihandle *statusLabel;
static Ihandle *filterText, *filterButton;

void showStatus(const char *line);
static int uiStopCb(Ihandle *ih);
static int uiStartCb(Ihandle *ih);
static void uiSetupModule(const Module &module, Ihandle *parent);

void init(int argc, char* argv[]) {
    IupOpen(&argc, &argv);

    // status and filter frame
    Ihandle *topVbox;
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

    // functionalities frame 
    Ihandle *bottomVbox;
    bottomFrame = IupFrame(
        bottomVbox = IupVbox(
            NULL
        )
    );
    IupSetAttribute(bottomFrame, "TITLE", "Functions");

    // setup module uis
    uiSetupModule(dropModule, bottomVbox);

    // dialog
    Ihandle *dialogVBox;
    dialog = IupDialog(
        dialogVBox = IupVbox(
            statusLabel,
            topFrame,
            bottomFrame
        )
    );

    IupSetAttribute(dialog, "TITLE", "clumpsy " CLUMPSY_VERSION);
    IupSetAttribute(dialog, "SIZE", "366x400"); // add padding manually to width
    IupSetAttribute(dialog, "RESIZE", "NO");

    // global layout settings to affect childrens
    IupSetAttribute(dialogVBox, "ALIGNMENT", "ACENTER");
    IupSetAttribute(dialogVBox, "CMARGIN", "4x4");
    IupSetAttribute(dialogVBox, "CGAP", "4x2");
    IupSetAttribute(dialogVBox, "PADDING", "6x2");

}

void startup() {
    // kickoff event loops
    IupShowXY(dialog, IUP_CENTER, IUP_CENTER);
    IupMainLoop();
}

void cleanup() {
    IupClose();
}

// ui logics
void showStatus(const char *line) {
    IupStoreAttribute(statusLabel, "TITLE", line); 
}

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

static void uiSetupModule(const Module &module, Ihandle *parent) {
    Ihandle *groupBox, *toggle, *controls;
    groupBox = IupHbox(
        toggle = IupToggle(module.name, NULL),
        IupFill(),
        controls = module.setupUIFunc(),
        NULL
    );
    IupSetAttribute(groupBox, "EXPAND", "HORIZONTAL");
    IupSetAttribute(groupBox, "ALIGNMENT", "ACENTER");
    IupSetAttribute(controls, "ALIGNMENT", "ACENTER");
    IupAppend(parent, groupBox);
}

int main(int argc, char* argv[]) {
    init(argc, argv);
    startup();
    cleanup();
    return 0;
}