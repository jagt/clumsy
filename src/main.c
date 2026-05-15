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
};

volatile short sendState = SEND_STATUS_NONE;

// global iup handlers
static Ihandle *dialog, *topFrame, *bottomFrame; 
static Ihandle *statusLabel;
static Ihandle *filterText, *filterButton;
static Ihandle *appFilterToggle;
static Ihandle *appFilterText;
static Ihandle *appFilterBrowseButton;
static Ihandle *appFilterModeList;
static Ihandle *appFilterControls;
Ihandle *filterSelectList;
// timer to update icons
static Ihandle *stateIcon;
static Ihandle *timer;
static Ihandle *timeout = NULL;
static BOOL dialogWasForeground = FALSE;

void showStatus(const char *line);
static int uiOnDialogShow(Ihandle *ih, int state);
static int uiStopCb(Ihandle *ih);
static int uiStartCb(Ihandle *ih);
static int uiTimerCb(Ihandle *ih);
static int uiTimeoutCb(Ihandle *ih);
static int uiListSelectCb(Ihandle *ih, char *text, int item, int state);
static int uiFilterTextCb(Ihandle *ih);
static int uiDialogFocusCb(Ihandle *ih);
static BOOL uiIsBlankText(const char *text);
static BOOL uiDialogIsForeground(void);
static void uiRefreshDialog(void);
static void uiSetAppFilterInputsLocked(BOOL locked);
static int uiAppFilterToggleCb(Ihandle *ih, int state);
static int uiAppFilterBrowseCb(Ihandle *ih);
static void uiBuildAppFilterConfig(AppFilterConfig *config);
static void uiSetupModule(Module *module, Ihandle *parent);

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
        filters[filtersSize].filterName = "ipv4 + ipv6 all";
        filters[filtersSize].filterValue = "ip or ipv6";
        filtersSize = 1;
    }
}

void init(int argc, char* argv[]) {
    UINT ix;
    Ihandle *topVbox, *bottomVbox, *dialogVBox, *controlHbox;
    Ihandle *appTargetHbox, *appModeHbox;
    Ihandle *noneIcon, *doingIcon, *errorIcon;
    char* arg_value = NULL;
    char *appArg = NULL;
    char *appModeArg = NULL;

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
            filterText = IupText(NULL),
            controlHbox = IupHbox(
                stateIcon = IupLabel(NULL),
                filterButton = IupButton("Start", NULL),
                IupFill(),
                IupLabel("Presets:  "),
                filterSelectList = IupList(NULL),
                NULL
            ),
            appFilterToggle = IupToggle("Limit to application", NULL),
            appFilterControls = IupVbox(
                appTargetHbox = IupHbox(
                    IupLabel("Application:"),
                    appFilterText = IupText(NULL),
                    appFilterBrowseButton = IupButton("Browse...", NULL),
                    NULL
                ),
                appModeHbox = IupHbox(
                    IupLabel("Match:"),
                    appFilterModeList = IupList(NULL),
                    NULL
                ),
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
    IupSetAttribute(topVbox, "EXPAND", "HORIZONTAL");
    IupSetAttribute(filterText, "EXPAND", "HORIZONTAL");
    IupSetCallback(filterText, "VALUECHANGED_CB", (Icallback)uiFilterTextCb);
    IupSetAttribute(filterButton, "PADDING", "8x");
    IupSetCallback(filterButton, "ACTION", uiStartCb);
    IupSetAttribute(topVbox, "NCMARGIN", "4x4");
    IupSetAttribute(topVbox, "NCGAP", "4x2");
    IupSetAttribute(controlHbox, "EXPAND", "HORIZONTAL");
    IupSetAttribute(controlHbox, "ALIGNMENT", "ACENTER");
    IupSetCallback(appFilterToggle, "ACTION", (Icallback)uiAppFilterToggleCb);
    IupSetAttribute(appFilterControls, "ACTIVE", "NO");
    IupSetAttribute(appFilterControls, "EXPAND", "HORIZONTAL");
    IupSetAttribute(appFilterControls, "NCGAP", "4x2");
    IupSetAttribute(appTargetHbox, "EXPAND", "HORIZONTAL");
    IupSetAttribute(appTargetHbox, "ALIGNMENT", "ACENTER");
    IupSetAttribute(appTargetHbox, "NCGAP", "4");
    IupSetAttribute(appModeHbox, "EXPAND", "HORIZONTAL");
    IupSetAttribute(appModeHbox, "ALIGNMENT", "ACENTER");
    IupSetAttribute(appModeHbox, "NCGAP", "4");
    IupSetAttribute(appFilterText, "EXPAND", "HORIZONTAL");
    IupSetAttribute(appFilterText, "VISIBLECOLUMNS", "36");
    IupSetAttribute(appFilterBrowseButton, "PADDING", "8x");
    IupSetCallback(appFilterBrowseButton, "ACTION", uiAppFilterBrowseCb);
    IupSetAttribute(appFilterModeList, "DROPDOWN", "YES");
    IupSetAttribute(appFilterModeList, "VISIBLECOLUMNS", "14");
    IupStoreAttribute(appFilterModeList, "1", "Process name");
    IupStoreAttribute(appFilterModeList, "2", "Full path");
    IupSetAttribute(appFilterModeList, "VALUE", "1");

    if (parameterized) {
        appArg = IupGetGlobal("app");
        appModeArg = IupGetGlobal("app-mode");
        if (appArg != NULL) {
            IupSetAttribute(appFilterToggle, "VALUE", "ON");
            IupSetAttribute(appFilterText, "VALUE", appArg);
            uiAppFilterToggleCb(appFilterToggle, 1);
        }
        if (appModeArg != NULL && (appModeArg[0] == 'n' || appModeArg[0] == 'N')) {
            IupSetAttribute(appFilterModeList, "VALUE", "1");
        } else if (appModeArg != NULL &&
                (appModeArg[0] == 'p' || appModeArg[0] == 'P' ||
                 appModeArg[0] == 'f' || appModeArg[0] == 'F')) {
            IupSetAttribute(appFilterModeList, "VALUE", "2");
        }
    }

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
    IupSetAttribute(bottomVbox, "NCMARGIN", "4x4");
    IupSetAttribute(bottomVbox, "NCGAP", "4x2");

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
    IupSetAttribute(dialog, "SIZE", "480x"); // add padding manually to width
    IupSetAttribute(dialog, "RESIZE", "NO");
    IupSetCallback(dialog, "SHOW_CB", (Icallback)uiOnDialogShow);
    IupSetCallback(dialog, "GETFOCUS_CB", (Icallback)uiDialogFocusCb);


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

static BOOL uiDialogIsForeground(void)
{
    HWND hWnd;
    HWND foreground;

    if (dialog == NULL)
    {
        return FALSE;
    }

    hWnd = (HWND)IupGetAttribute(dialog, "HWND");
    foreground = GetForegroundWindow();
    return hWnd != NULL && foreground != NULL &&
        (foreground == hWnd || IsChild(hWnd, foreground));
}

static void uiRefreshDialog(void)
{
    HWND hWnd;

    if (dialog == NULL)
    {
        return;
    }

    IupRefresh(dialog);
    hWnd = (HWND)IupGetAttribute(dialog, "HWND");
    if (hWnd != NULL)
    {
        RedrawWindow(hWnd, NULL, NULL,
                     RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
    }
}

static int uiDialogFocusCb(Ihandle* ih)
{
    UNREFERENCED_PARAMETER(ih);
    uiRefreshDialog();
    dialogWasForeground = TRUE;
    return IUP_DEFAULT;
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
    AppFilterConfig appConfig;
    UNREFERENCED_PARAMETER(ih);

    uiBuildAppFilterConfig(&appConfig);
    if (appConfig.enabled && appConfig.targets[0] == '\0') {
        showStatus("Enter an application name or path before starting the application filter.");
        return IUP_DEFAULT;
    }

    if (divertStart(IupGetAttribute(filterText, "VALUE"), &appConfig, buf) == 0) {
        showStatus(buf);
        return IUP_DEFAULT;
    }

    // successfully started
    if (appConfig.enabled) {
        showStatus("Started filtering for the selected application. Enable functionalities to take effect.");
    } else {
        showStatus("Started filtering. Enable functionalities to take effect.");
    }
    IupSetAttribute(filterText, "READONLY", "YES");
    IupSetAttribute(appFilterToggle, "ACTIVE", "NO");
    uiSetAppFilterInputsLocked(TRUE);
    IupSetAttribute(filterButton, "TITLE", "Stop");
    IupSetCallback(filterButton, "ACTION", uiStopCb);
    IupSetAttribute(timer, "RUN", "YES");
    uiRefreshDialog();

    return IUP_DEFAULT;
}

static int uiStopCb(Ihandle *ih) {
    int ix;
    UNREFERENCED_PARAMETER(ih);
    
    // try stopping
    IupSetAttribute(filterButton, "ACTIVE", "NO");
    IupFlush(); // flush to show disabled state
    divertStop();

    IupSetAttribute(filterText, "READONLY", "NO");
    IupSetAttribute(appFilterToggle, "ACTIVE", "YES");
    uiSetAppFilterInputsLocked(FALSE);
    uiAppFilterToggleCb(appFilterToggle, IupGetInt(appFilterToggle, "VALUE"));
    IupSetAttribute(filterButton, "TITLE", "Start");
    IupSetAttribute(filterButton, "ACTIVE", "YES");
    IupSetCallback(filterButton, "ACTION", uiStartCb);

    // stop timer and clean up icons
    IupSetAttribute(timer, "RUN", "NO");
    for (ix = 0; ix < MODULE_CNT; ++ix) {
        modules[ix]->processTriggered = 0; // use = here since is threads already stopped
        IupSetAttribute(modules[ix]->iconHandle, "IMAGE", "none_icon");
    }
    sendState = SEND_STATUS_NONE;
    IupSetAttribute(stateIcon, "IMAGE", "none_icon");

    showStatus("Stopped. To begin again, edit criteria and click Start.");
    uiRefreshDialog();
    return IUP_DEFAULT;
}

static BOOL uiIsBlankText(const char* text)
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

static void uiSetAppFilterInputsLocked(BOOL locked)
{
    IupSetAttribute(appFilterText, "READONLY", locked ? "YES" : "NO");
    IupSetAttribute(appFilterBrowseButton, "ACTIVE", locked ? "NO" : "YES");
    IupSetAttribute(appFilterModeList, "ACTIVE", locked ? "NO" : "YES");
}

static int uiAppFilterToggleCb(Ihandle* ih, int state)
{
    UNREFERENCED_PARAMETER(ih);
    IupSetAttribute(appFilterControls, "ACTIVE", state ? "YES" : "NO");
    uiRefreshDialog();
    return IUP_DEFAULT;
}

static int uiAppFilterBrowseCb(Ihandle* ih)
{
    Ihandle* fileDlg;
    const char* path;
    int status;

    UNREFERENCED_PARAMETER(ih);

    fileDlg = IupFileDlg();
    IupSetAttribute(fileDlg, "DIALOGTYPE", "OPEN");
    IupSetAttribute(fileDlg, "TITLE", "Select application executable");
    IupSetAttribute(fileDlg, "EXTFILTER", "Executable files|*.exe|All files|*.*|");
    IupSetAttributeHandle(fileDlg, "PARENTDIALOG", dialog);
    IupPopup(fileDlg, IUP_CENTERPARENT, IUP_CENTERPARENT);

    status = IupGetInt(fileDlg, "STATUS");
    if (status != -1)
    {
        path = IupGetAttribute(fileDlg, "VALUE");
        if (!uiIsBlankText(path))
        {
            IupStoreAttribute(appFilterText, "VALUE", path);
            IupSetAttribute(appFilterModeList, "VALUE", "2");
        }
    }

    IupDestroy(fileDlg);
    return IUP_DEFAULT;
}

static void uiBuildAppFilterConfig(AppFilterConfig* config)
{
    const char* targetsText;

    AppFilterDefaultConfig(config);
    config->enabled = IupGetInt(appFilterToggle, "VALUE") ? TRUE : FALSE;
    config->mode = IupGetInt(appFilterModeList, "VALUE") == 2
                       ? APP_FILTER_MODE_FULL_PATH
                       : APP_FILTER_MODE_PROCESS_NAME;
    config->includeChildProcesses = TRUE;

    targetsText = IupGetAttribute(appFilterText, "VALUE");
    if (!uiIsBlankText(targetsText))
    {
        strncpy(config->targets, targetsText, APP_FILTER_TARGETS_BUFSIZE - 1);
        config->targets[APP_FILTER_TARGETS_BUFSIZE - 1] = '\0';
    }
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

    return IUP_DEFAULT;
}

static int uiTimerCb(Ihandle *ih) {
    int ix;
    BOOL dialogIsForeground;
    UNREFERENCED_PARAMETER(ih);

    dialogIsForeground = uiDialogIsForeground();
    if (dialogIsForeground && !dialogWasForeground)
    {
        uiRefreshDialog();
    }
    dialogWasForeground = dialogIsForeground;

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

    if (AppFilterIsEnabled())
    {
        AppFilterStats stats;
        char statusBuf[MSG_BUFSIZE];
        AppFilterGetStats(&stats);
        sprintf(statusBuf,
                "Application filter: target PIDs %ld, target flows %ld, UDP endpoints %ld, affected %ld, passed %ld, unknown %ld.",
                stats.targetPidCount, stats.targetFlowCount, stats.targetEndpointCount,
                stats.affectedPackets, stats.passedUnmatchedPackets, stats.unknownPackets);
        showStatus(statusBuf);
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
    Ihandle *groupBox, *toggle, *controls, *icon;
    groupBox = IupHbox(
        icon = IupLabel(NULL),
        toggle = IupToggle(module->displayName, NULL),
        IupFill(),
        controls = module->setupUIFunc(),
        NULL
    );
    IupSetAttribute(groupBox, "EXPAND", "HORIZONTAL");
    IupSetAttribute(groupBox, "ALIGNMENT", "ACENTER");
    IupSetAttribute(controls, "ALIGNMENT", "ACENTER");
    IupAppend(parent, groupBox);

    // set controls as attribute to toggle and enable toggle callback
    IupSetCallback(toggle, "ACTION", (Icallback)uiToggleControls);
    IupSetAttribute(toggle, CONTROLS_HANDLE, (char*)controls);
    IupSetAttribute(toggle, SYNCED_VALUE, (char*)module->enabledFlag);
    IupSetAttribute(controls, "ACTIVE", "NO"); // startup as inactive
    IupSetAttribute(controls, "NCGAP", "4"); // startup as inactive

    // set default icon
    IupSetAttribute(icon, "IMAGE", "none_icon");
    IupSetAttribute(icon, "PADDING", "4x");
    module->iconHandle = icon;

    // parameterize toggle
    if (parameterized) {
        setFromParameter(toggle, "VALUE", module->shortName);
    }
}

int main(int argc, char* argv[]) {
    LOG("Is Run As Admin: %d", IsRunAsAdmin());
    LOG("Is Elevated: %d", IsElevated());
    init(argc, argv);
    startup();
    cleanup();
    return 0;
}
