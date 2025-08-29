#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <Windows.h>
#include "iup.h"
#include "common.h"

// ! the order decides which module get processed first
Module* modules[MODULE_CNT] = {
    &lagModule,
    &dropModule,
    &throttleModule,
    &dupModule,
    &oodModule,
    &tamperModule,
    &resetModule,
	&bandwidthModule,
	&lengthModule,
	&tlsModule
};

// Excluded from main UI but processed separately
Module* hiddenModules[] = {
    &statsModule,
    &loggingModule,
    &automationModule,
    &protocolModule,
    &profilesModule,
};
#define HIDDEN_MODULE_CNT 5

// Helper function to process all modules (visible + hidden)
void processAllModules(void (*func)(Module*, int)) {
    int ix;
    // Process visible modules
    for (ix = 0; ix < MODULE_CNT; ++ix) {
        func(modules[ix], ix);
    }
    // Process hidden modules  
    for (ix = 0; ix < HIDDEN_MODULE_CNT; ++ix) {
        func(hiddenModules[ix], MODULE_CNT + ix);
    }
}

volatile short sendState = SEND_STATUS_NONE;

// global iup handlers
static Ihandle *dialog, *topFrame, *bottomFrame; 
static Ihandle *statusLabel;
static Ihandle *filterText, *filterButton;
Ihandle *filterSelectList;
// timer to update icons
static Ihandle *stateIcon;
static Ihandle *timer;
static Ihandle *timeout = NULL;
// side panels
static Ihandle *logDialog = NULL, *statsDialog = NULL, *automationDialog = NULL;
static volatile short logPanelVisible = 0, statsPanelVisible = 0, automationPanelVisible = 0;
// settings tabs
static Ihandle *settingsTabs = NULL;
static Ihandle *loggingToggle = NULL, *statsToggle = NULL, *automationToggle = NULL;

void showStatus(const char *line);
static int uiOnDialogShow(Ihandle *ih, int state);
static int uiStopCb(Ihandle *ih);
static int uiStartCb(Ihandle *ih);
static int uiTimerCb(Ihandle *ih);
static int uiTimeoutCb(Ihandle *ih);
static int uiListSelectCb(Ihandle *ih, char *text, int item, int state);
static int uiFilterTextCb(Ihandle *ih);
static void uiSetupModule(Module *module, Ihandle *parent);
static int uiToggleLogPanel(Ihandle *ih, int state);
static int uiToggleStatsPanel(Ihandle *ih, int state);
static int uiToggleAutomationPanel(Ihandle *ih, int state);
static void uiCreateLogPanel(void);
static void uiCreateStatsPanel(void);
static void uiCreateAutomationPanel(void);

// serializing config files using a stupid custom format
#define CONFIG_FILE "config.txt"
#define CONFIG_MAX_RECORDS 64
#define CONFIG_BUF_SIZE 4096
typedef struct {
    char* filterName;
    char* filterValue;
} filterRecord;
UINT filtersSize;
filterRecord filters[CONFIG_MAX_RECORDS] = {0};
char configBuf[CONFIG_BUF_SIZE+2]; // add some padding to write \n
BOOL parameterized = 0; // parameterized flag, means reading args from command line

// loading up filters and fill in
void loadConfig() {
    char path[MSG_BUFSIZE];
    char *p;
    FILE *f;
    GetModuleFileName(NULL, path, MSG_BUFSIZE);
    LOG("Executable path: %s", path);
    p = strrchr(path, '\\');
    if (p == NULL) p = strrchr(path, '/'); // holy shit
    strcpy(p+1, CONFIG_FILE);
    LOG("Config path: %s", path);
    f = fopen(path, "r");
    if (f) {
        size_t len;
        char *current, *last;
        len = fread(configBuf, sizeof(char), CONFIG_BUF_SIZE, f);
        if (len == CONFIG_BUF_SIZE) {
            LOG("Config file is larger than %d bytes, get truncated.", CONFIG_BUF_SIZE);
        }
        // always patch in a newline at the end to ease parsing
        configBuf[len] = '\n';
        configBuf[len+1] = '\0';

        // parse out the kv pairs. isn't quite safe
        filtersSize = 0;
        last = current = configBuf;
        do {
            // eat up empty lines
EAT_SPACE:  while (isspace(*current)) { ++current; }
            if (*current == '#') {
                current = strchr(current, '\n');
                if (!current) break;
                current = current + 1;
                goto EAT_SPACE;
            }

            // now we can start
            last = current;
            current = strchr(last, ':');
            if (!current) break;
            *current = '\0';
            filters[filtersSize].filterName = last;
            current += 1;
            while (isspace(*current)) { ++current; } // eat potential space after :
            last = current;
            current = strchr(last, '\n');
            if (!current) break;
            filters[filtersSize].filterValue = last;
            *current = '\0';
            if (*(current-1) == '\r') *(current-1) = 0;
            last = current = current + 1;
            ++filtersSize;
        } while (last && last - configBuf < CONFIG_BUF_SIZE);
        LOG("Loaded %u records.", filtersSize);
    }

    if (!f || filtersSize == 0)
    {
        LOG("Failed to load from config. Fill in a simple one.");
        // config is missing or ill-formed. fill in some simple ones
        filters[filtersSize].filterName = "loopback packets";
        filters[filtersSize].filterValue = "outbound and ip.DstAddr >= 127.0.0.1 and ip.DstAddr <= 127.255.255.255";
        filtersSize = 1;
    }
}

void init(int argc, char* argv[]) {
    UINT ix;
    Ihandle *topVbox, *bottomVbox, *dialogVBox, *controlHbox;
    Ihandle *noneIcon, *doingIcon, *errorIcon;
    char* arg_value = NULL;

    // fill in config
    loadConfig();

    // iup inits
    IupOpen(&argc, &argv);

    // this is so easy to get wrong so it's pretty worth noting in the program
    statusLabel = IupLabel("NOTICE: When capturing localhost (loopback) packets, you CAN'T include inbound criteria.\n"
        "Filters like 'udp' need to be 'udp and outbound' to work. See readme for more info.");
    IupSetAttribute(statusLabel, "EXPAND", "HORIZONTAL");
    IupSetAttribute(statusLabel, "PADDING", "8x8");

    topFrame = IupFrame(
        topVbox = IupVbox(
            // Settings tabs
            settingsTabs = IupHbox(
                IupLabel("Panels:"),
                loggingToggle = IupToggle("Logs", NULL),
                statsToggle = IupToggle("Stats", NULL),
                automationToggle = IupToggle("Auto", NULL),
                IupFill(),
                NULL
            ),
            filterText = IupText(NULL),
            controlHbox = IupHbox(
                stateIcon = IupLabel(NULL),
                filterButton = IupButton("Start", NULL),
                IupFill(),
                IupLabel("Presets:  "),
                filterSelectList = IupList(NULL),
                NULL
            ),
            NULL
        )
    );

    // parse arguments and set globals *before* setting up UI.
    // arguments can be read and set after callbacks are setup
    // FIXME as Release is built as WindowedApp, stdout/stderr won't show
    LOG("argc: %d", argc);
    if (argc > 1) {
        if (!parseArgs(argc, argv)) {
            fprintf(stderr, "invalid argument count. ensure you're using options as \"--drop on\"");
            exit(-1); // fail fast.
        }
        parameterized = 1;
    }

    IupSetAttribute(topFrame, "TITLE", "Filtering");
    IupSetAttribute(topFrame, "EXPAND", "HORIZONTAL");
    IupSetAttribute(filterText, "EXPAND", "HORIZONTAL");
    IupSetCallback(filterText, "VALUECHANGED_CB", (Icallback)uiFilterTextCb);
    IupSetAttribute(filterButton, "PADDING", "8x");
    IupSetCallback(filterButton, "ACTION", uiStartCb);
    
    // Setup panel toggles
    IupSetCallback(loggingToggle, "ACTION", (Icallback)uiToggleLogPanel);
    IupSetCallback(statsToggle, "ACTION", (Icallback)uiToggleStatsPanel);
    IupSetCallback(automationToggle, "ACTION", (Icallback)uiToggleAutomationPanel);
    IupSetAttribute(loggingToggle, "VALUE", "OFF");
    IupSetAttribute(statsToggle, "VALUE", "OFF");
    IupSetAttribute(automationToggle, "VALUE", "OFF");
    
    IupSetAttribute(topVbox, "NCMARGIN", "4x4");
    IupSetAttribute(topVbox, "NCGAP", "4x2");
    IupSetAttribute(controlHbox, "ALIGNMENT", "ACENTER");

    // setup state icon
    IupSetAttribute(stateIcon, "IMAGE", "none_icon");
    IupSetAttribute(stateIcon, "PADDING", "4x");

    // fill in options and setup callback
    IupSetAttribute(filterSelectList, "VISIBLECOLUMNS", "24");
    IupSetAttribute(filterSelectList, "DROPDOWN", "YES");
    for (ix = 0; ix < filtersSize; ++ix) {
        char ixBuf[4];
        sprintf(ixBuf, "%d", ix+1); // ! staring from 1, following lua indexing
        IupStoreAttribute(filterSelectList, ixBuf, filters[ix].filterName);
    }
    IupSetAttribute(filterSelectList, "VALUE", "1");
    IupSetCallback(filterSelectList, "ACTION", (Icallback)uiListSelectCb);
    // set filter text value since the callback won't take effect before main loop starts
    IupSetAttribute(filterText, "VALUE", filters[0].filterValue);

    // functionalities frame 
    bottomFrame = IupFrame(
        bottomVbox = IupVbox(
            NULL
        )
    );
    IupSetAttribute(bottomFrame, "TITLE", "Functions");
    IupSetAttribute(bottomFrame, "ALIGNMENT", "ALEFT");
    IupSetAttribute(bottomVbox, "NCMARGIN", "4x4");
    IupSetAttribute(bottomVbox, "NCGAP", "4x2");
    IupSetAttribute(bottomVbox, "ALIGNMENT", "ALEFT");   // Выравнивание по левой стороне


    // create icons
    noneIcon = IupImage(8, 8, icon8x8);
    doingIcon = IupImage(8, 8, icon8x8);
    errorIcon = IupImage(8, 8, icon8x8);
    IupSetAttribute(noneIcon, "0", "BGCOLOR");
    IupSetAttribute(noneIcon, "1", "224 224 224");
    IupSetAttribute(doingIcon, "0", "BGCOLOR");
    IupSetAttribute(doingIcon, "1", "109 170 44");
    IupSetAttribute(errorIcon, "0", "BGCOLOR");
    IupSetAttribute(errorIcon, "1", "208 70 72");
    IupSetHandle("none_icon", noneIcon);
    IupSetHandle("doing_icon", doingIcon);
    IupSetHandle("error_icon", errorIcon);

    // setup module uis
    for (ix = 0; ix < MODULE_CNT; ++ix) {
        uiSetupModule(*(modules+ix), bottomVbox);
    }

    // dialog
    dialog = IupDialog(
        dialogVBox = IupVbox(
            topFrame,
            bottomFrame,
            statusLabel,
            NULL
        )
    );

    IupSetAttribute(dialog, "TITLE", "clumsy " CLUMSY_VERSION);
    IupSetAttribute(dialog, "SIZE", "480x"); // Minimum width, height will adjust
    IupSetAttribute(dialog, "RESIZE", "YES");
    IupSetAttribute(dialog, "MINBOX", "YES");
    IupSetAttribute(dialog, "MAXBOX", "YES");
    IupSetCallback(dialog, "SHOW_CB", (Icallback)uiOnDialogShow);


    // global layout settings to affect childrens
    IupSetAttribute(dialogVBox, "ALIGNMENT", "ACENTER");
    IupSetAttribute(dialogVBox, "NCMARGIN", "4x4");
    IupSetAttribute(dialogVBox, "NCGAP", "4x2");

    // setup timer
    timer = IupTimer();
    IupSetAttribute(timer, "TIME", STR(ICON_UPDATE_MS));
    IupSetCallback(timer, "ACTION_CB", uiTimerCb);

    // setup timeout of program
    arg_value = IupGetGlobal("timeout");
    if(arg_value != NULL)
    {
        char valueBuf[16];
        sprintf(valueBuf, "%s000", arg_value);  // convert from seconds to milliseconds

        timeout = IupTimer();
        IupStoreAttribute(timeout, "TIME", valueBuf);
        IupSetCallback(timeout, "ACTION_CB", uiTimeoutCb);
        IupSetAttribute(timeout, "RUN", "YES");
    }
}

void startup() {
    // initialize seed
    srand((unsigned int)time(NULL));

    // kickoff event loops
    IupShowXY(dialog, IUP_CENTER, IUP_CENTER);
    IupMainLoop();
    // ! main loop won't return until program exit
}

void cleanup() {

    IupDestroy(timer);
    if (timeout) {
        IupDestroy(timeout);
    }

    IupClose();
    endTimePeriod(); // try close if not closing
}

// ui logics
void showStatus(const char *line) {
    IupStoreAttribute(statusLabel, "TITLE", line); 
}

// in fact only 32bit binary would run on 64 bit os
// if this happens pop out message box and exit
static BOOL check32RunningOn64(HWND hWnd) {
    BOOL is64ret;
    // consider IsWow64Process return value
    if (IsWow64Process(GetCurrentProcess(), &is64ret) && is64ret) {
        MessageBox(hWnd, (LPCSTR)"You're running 32bit clumsy on 64bit Windows, which wouldn't work. Please use the 64bit clumsy version.",
            (LPCSTR)"Aborting", MB_OK);
        return TRUE;
    }
    return FALSE;
}

static BOOL checkIsRunning() {
    //It will be closed and destroyed when programm terminates (according to MSDN).
    HANDLE hStartEvent = CreateEventW(NULL, FALSE, FALSE, L"Global\\CLUMSY_IS_RUNNING_EVENT_NAME");

    if (hStartEvent == NULL)
        return TRUE;

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hStartEvent);
        hStartEvent = NULL;
        return TRUE;
    }

    return FALSE;
}


static int uiOnDialogShow(Ihandle *ih, int state) {
    // only need to process on show
    HWND hWnd;
    BOOL exit;
    HICON icon;
    HINSTANCE hInstance;
    if (state != IUP_SHOW) return IUP_DEFAULT;
    hWnd = (HWND)IupGetAttribute(ih, "HWND");
    hInstance = GetModuleHandle(NULL);

    // set application icon
    icon = LoadIcon(hInstance, "CLUMSY_ICON");
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);

    exit = checkIsRunning();
    if (exit) {
        MessageBox(hWnd, (LPCSTR)"Theres' already an instance of clumsy running.",
            (LPCSTR)"Aborting", MB_OK);
        return IUP_CLOSE;
    }

#ifdef _WIN32
    exit = check32RunningOn64(hWnd);
    if (exit) {
        return IUP_CLOSE;
    }
#endif

    // try elevate and decides whether to exit
    exit = tryElevate(hWnd, parameterized);

    if (!exit && parameterized) {
        setFromParameter(filterText, "VALUE", "filter");
        LOG("is parameterized, start filtering upon execution.");
        uiStartCb(filterButton);
    }

    return exit ? IUP_CLOSE : IUP_DEFAULT;
}

static int uiStartCb(Ihandle *ih) {
    char buf[MSG_BUFSIZE];
    UNREFERENCED_PARAMETER(ih);
    if (divertStart(IupGetAttribute(filterText, "VALUE"), buf) == 0) {
        showStatus(buf);
        return IUP_DEFAULT;
    }

    // successfully started
    showStatus("Started filtering. Enable functionalities to take effect.");
    IupSetAttribute(filterText, "ACTIVE", "NO");
    IupSetAttribute(filterButton, "TITLE", "Stop");
    IupSetCallback(filterButton, "ACTION", uiStopCb);
    IupSetAttribute(timer, "RUN", "YES");

    return IUP_DEFAULT;
}

static int uiStopCb(Ihandle *ih) {
    int ix;
    UNREFERENCED_PARAMETER(ih);
    
    // try stopping
    IupSetAttribute(filterButton, "ACTIVE", "NO");
    IupFlush(); // flush to show disabled state
    divertStop();

    IupSetAttribute(filterText, "ACTIVE", "YES");
    IupSetAttribute(filterButton, "TITLE", "Start");
    IupSetAttribute(filterButton, "ACTIVE", "YES");
    IupSetCallback(filterButton, "ACTION", uiStartCb);

    // stop timer and clean up icons
    IupSetAttribute(timer, "RUN", "NO");
    for (ix = 0; ix < MODULE_CNT; ++ix) {
        modules[ix]->processTriggered = 0; // use = here since is threads already stopped
        IupSetAttribute(modules[ix]->iconHandle, "IMAGE", "none_icon");
    }
    // clean up hidden modules
    for (ix = 0; ix < HIDDEN_MODULE_CNT; ++ix) {
        hiddenModules[ix]->processTriggered = 0;
    }
    sendState = SEND_STATUS_NONE;
    IupSetAttribute(stateIcon, "IMAGE", "none_icon");

    showStatus("Stopped. To begin again, edit criteria and click Start.");
    return IUP_DEFAULT;
}

static int uiToggleControls(Ihandle *ih, int state) {
    Ihandle *controls = (Ihandle*)IupGetAttribute(ih, CONTROLS_HANDLE);
    short *target = (short*)IupGetAttribute(ih, SYNCED_VALUE);
    int controlsActive = IupGetInt(controls, "ACTIVE");
    
    if (controlsActive && !state) {
        IupSetAttribute(controls, "ACTIVE", "NO");
        InterlockedExchange16(target, I2S(state));
    } else if (!controlsActive && state) {
        IupSetAttribute(controls, "ACTIVE", "YES");
        InterlockedExchange16(target, I2S(state));
    }

    // Refresh the dialog layout to ensure proper sizing
    IupRefresh(dialog);

    return IUP_DEFAULT;
}

static int uiTimerCb(Ihandle *ih) {
    int ix;
    UNREFERENCED_PARAMETER(ih);
    for (ix = 0; ix < MODULE_CNT; ++ix) {
        if (modules[ix]->processTriggered) {
            IupSetAttribute(modules[ix]->iconHandle, "IMAGE", "doing_icon");
            InterlockedAnd16(&(modules[ix]->processTriggered), 0);
        } else {
            IupSetAttribute(modules[ix]->iconHandle, "IMAGE", "none_icon");
        }
    }

    // update global send status icon
    switch (sendState)
    {
    case SEND_STATUS_NONE:
        IupSetAttribute(stateIcon, "IMAGE", "none_icon");
        break;
    case SEND_STATUS_SEND:
        IupSetAttribute(stateIcon, "IMAGE", "doing_icon");
        InterlockedAnd16(&sendState, SEND_STATUS_NONE);
        break;
    case SEND_STATUS_FAIL:
        IupSetAttribute(stateIcon, "IMAGE", "error_icon");
        InterlockedAnd16(&sendState, SEND_STATUS_NONE);
        break;
    }

    return IUP_DEFAULT;
}

static int uiTimeoutCb(Ihandle *ih) {
    UNREFERENCED_PARAMETER(ih);
    return IUP_CLOSE;
 }

static int uiListSelectCb(Ihandle *ih, char *text, int item, int state) {
    UNREFERENCED_PARAMETER(text);
    UNREFERENCED_PARAMETER(ih);
    if (state == 1) {
        IupSetAttribute(filterText, "VALUE", filters[item-1].filterValue);
    }
    return IUP_DEFAULT;
}

static int uiFilterTextCb(Ihandle *ih)  {
    UNREFERENCED_PARAMETER(ih);
    // unselect list
    IupSetAttribute(filterSelectList, "VALUE", "0");
    return IUP_DEFAULT;
}

static void uiSetupModule(Module *module, Ihandle *parent) {
    Ihandle *groupBox, *toggle, *controls, *icon, *nameLabel;
    char formattedName[64];
    
    // Create formatted name with fixed width (30 characters)
    snprintf(formattedName, sizeof(formattedName), "%-30s", module->displayName);
    
    groupBox = IupHbox(
        icon = IupLabel(NULL),
        nameLabel = IupLabel(formattedName),  // Use formatted name label instead of toggle text
        toggle = IupToggle("", NULL),        // Empty toggle text since we have the label
        controls = module->setupUIFunc(),
        NULL
    );
    IupSetAttribute(groupBox, "EXPAND", "HORIZONTAL");
    IupSetAttribute(groupBox, "ALIGNMENT", "ACENTER");
    IupSetAttribute(controls, "ALIGNMENT", "ACENTER");
    IupAppend(parent, groupBox);

    // Set fixed width for the name label (approximately 30 characters)
    IupSetAttribute(nameLabel, "SIZE", "120x");  // Adjust size as needed for 30 characters
    
    // set controls as attribute to toggle and enable toggle callback
    IupSetCallback(toggle, "ACTION", (Icallback)uiToggleControls);
    IupSetAttribute(toggle, CONTROLS_HANDLE, (char*)controls);
    IupSetAttribute(toggle, SYNCED_VALUE, (char*)module->enabledFlag);
    IupSetAttribute(controls, "ACTIVE", "NO"); // startup as inactive
    IupSetAttribute(controls, "NCGAP", "4");

    // set default icon
    IupSetAttribute(icon, "IMAGE", "none_icon");
    IupSetAttribute(icon, "PADDING", "4x");
    module->iconHandle = icon;

    // parameterize toggle
    if (parameterized) {
        setFromParameter(toggle, "VALUE", module->shortName);
    }
}

// Create side panels for hidden modules
static void uiCreateLogPanel(void) {
    if (logDialog) return; // Already created
    
    // Create dialog with the actual logging module UI
    Ihandle *loggingUI = loggingModule.setupUIFunc();
    
    logDialog = IupDialog(
        IupVbox(
            IupLabel("Enhanced Logging Module"),
            loggingUI,
            NULL
        )
    );
    
    IupSetAttribute(logDialog, "TITLE", "clumsy - Logging");
    IupSetAttribute(logDialog, "RESIZE", "YES");
    IupSetAttribute(logDialog, "MINSIZE", "400x300");
    IupSetAttribute(logDialog, "TOPMOST", "YES");
}

static void uiCreateStatsPanel(void) {
    if (statsDialog) return; // Already created
    
    // Create dialog with the actual stats module UI
    Ihandle *statsUI = statsModule.setupUIFunc();
    
    statsDialog = IupDialog(
        IupVbox(
            IupLabel("Network Statistics Module"),
            statsUI,
            NULL
        )
    );
    
    IupSetAttribute(statsDialog, "TITLE", "clumsy - Statistics");
    IupSetAttribute(statsDialog, "RESIZE", "YES");
    IupSetAttribute(statsDialog, "MINSIZE", "400x250");
    IupSetAttribute(statsDialog, "TOPMOST", "YES");
}

static void uiCreateAutomationPanel(void) {
    if (automationDialog) return; // Already created
    
    // Create dialog with the actual automation module UI
    Ihandle *automationUI = automationModule.setupUIFunc();
    
    automationDialog = IupDialog(
        IupVbox(
            IupLabel("Automation & Scripting Module"),
            automationUI,
            NULL
        )
    );
    
    IupSetAttribute(automationDialog, "TITLE", "clumsy - Automation");
    IupSetAttribute(automationDialog, "RESIZE", "YES");
    IupSetAttribute(automationDialog, "MINSIZE", "500x400");
    IupSetAttribute(automationDialog, "TOPMOST", "YES");
}

static int uiToggleLogPanel(Ihandle *ih, int state) {
    UNREFERENCED_PARAMETER(ih);
    
    if (state) {
        if (!logDialog) uiCreateLogPanel();
        
        // Enable logging module
        *(loggingModule.enabledFlag) = 1;
        
        // Position panel to the right of main window
        int x, y;
        IupGetIntInt(dialog, "SCREENPOSITION", &x, &y);
        int width = IupGetInt(dialog, "RASTERSIZE");
        IupShowXY(logDialog, x + width + 10, y);
        logPanelVisible = 1;
    } else {
        // Disable logging module
        *(loggingModule.enabledFlag) = 0;
        
        if (logDialog) {
            IupHide(logDialog);
            logPanelVisible = 0;
        }
    }
    
    return IUP_DEFAULT;
}

static int uiToggleStatsPanel(Ihandle *ih, int state) {
    UNREFERENCED_PARAMETER(ih);
    
    if (state) {
        if (!statsDialog) uiCreateStatsPanel();
        
        // Enable stats module
        *(statsModule.enabledFlag) = 1;
        
        // Position panel to the right of main window
        int x, y;
        IupGetIntInt(dialog, "SCREENPOSITION", &x, &y);
        int width = IupGetInt(dialog, "RASTERSIZE");
        int offsetY = logPanelVisible ? 320 : 0; // Stack below log panel if visible
        IupShowXY(statsDialog, x + width + 10, y + offsetY);
        statsPanelVisible = 1;
    } else {
        // Disable stats module
        *(statsModule.enabledFlag) = 0;
        
        if (statsDialog) {
            IupHide(statsDialog);
            statsPanelVisible = 0;
        }
    }
    
    return IUP_DEFAULT;
}

static int uiToggleAutomationPanel(Ihandle *ih, int state) {
    UNREFERENCED_PARAMETER(ih);
    
    if (state) {
        if (!automationDialog) uiCreateAutomationPanel();
        
        // Enable automation module
        *(automationModule.enabledFlag) = 1;
        
        // Position panel to the right of main window
        int x, y;
        IupGetIntInt(dialog, "SCREENPOSITION", &x, &y);
        int width = IupGetInt(dialog, "RASTERSIZE");
        int offsetY = 0;
        if (logPanelVisible) offsetY += 320;
        if (statsPanelVisible) offsetY += 270;
        IupShowXY(automationDialog, x + width + 10, y + offsetY);
        automationPanelVisible = 1;
    } else {
        // Disable automation module
        *(automationModule.enabledFlag) = 0;
        
        if (automationDialog) {
            IupHide(automationDialog);
            automationPanelVisible = 0;
        }
    }
    
    return IUP_DEFAULT;
}



int main(int argc, char* argv[]) {
    LOG("Is Run As Admin: %d", IsRunAsAdmin());
    LOG("Is Elevated: %d", IsElevated());
    init(argc, argv);
    startup();
    cleanup();
    return 0;
}
