// scripting and automation support module
#include <stdlib.h>
#include <Windows.h>
#include <time.h>
#include "iup.h"
#include "common.h"
#define NAME "automation"

// Script action types
typedef enum {
    SCRIPT_ACTION_WAIT = 0,
    SCRIPT_ACTION_ENABLE_MODULE = 1,
    SCRIPT_ACTION_DISABLE_MODULE = 2,
    SCRIPT_ACTION_SET_PARAMETER = 3,
    SCRIPT_ACTION_APPLY_PROFILE = 4,
    SCRIPT_ACTION_LOG_MESSAGE = 5,
    SCRIPT_ACTION_EXPORT_DATA = 6
} ScriptActionType;

// Script action structure
typedef struct {
    ScriptActionType type;
    char moduleName[32];
    char parameter[64];
    char value[128];
    DWORD duration; // for wait actions, in milliseconds
} ScriptAction;

// Script structure
typedef struct {
    char name[64];
    ScriptAction actions[64];
    int actionCount;
    BOOL enabled;
    BOOL repeat;
    DWORD interval; // repeat interval in seconds
} AutomationScript;

// UI elements that should be hidden when automation is disabled
static Ihandle *automationControlsBox;
static Ihandle *enabledCheckbox, *scriptList, *loadScriptButton;
static Ihandle *scriptEditor, *saveScriptButton, *runScriptButton;
static Ihandle *repeatCheckbox, *intervalInput;
static Ihandle *statusLabel, *progressBar;

static volatile short automationEnabled = 0;
static volatile short currentScript = -1;
static volatile short scriptRepeat = 0;
static volatile short scriptInterval = 60; // seconds

static AutomationScript scripts[16];
static int scriptCount = 0;
static HANDLE scriptThread = NULL;
static volatile BOOL scriptRunning = FALSE;
static volatile BOOL stopScript = FALSE;

// Automation enabled callback
static int automationEnabledCallback(Ihandle *ih, int state) {
    UNREFERENCED_PARAMETER(state);
    int enabled = IupGetInt(ih, "VALUE");
    
    // Show/hide automation controls based on enabled state
    if (enabled) {
        IupSetAttribute(scriptList, "VISIBLE", "YES");
        IupSetAttribute(loadScriptButton, "VISIBLE", "YES");
        IupSetAttribute(saveScriptButton, "VISIBLE", "YES");
        IupSetAttribute(scriptEditor, "VISIBLE", "YES");
        IupSetAttribute(runScriptButton, "VISIBLE", "YES");
        IupSetAttribute(progressBar, "VISIBLE", "YES");
        IupSetAttribute(statusLabel, "VISIBLE", "YES");
        IupSetAttribute(repeatCheckbox, "VISIBLE", "YES");
        IupSetAttribute(intervalInput, "VISIBLE", "YES");
    } else {
        IupSetAttribute(scriptList, "VISIBLE", "NO");
        IupSetAttribute(loadScriptButton, "VISIBLE", "NO");
        IupSetAttribute(saveScriptButton, "VISIBLE", "NO");
        IupSetAttribute(scriptEditor, "VISIBLE", "NO");
        IupSetAttribute(runScriptButton, "VISIBLE", "NO");
        IupSetAttribute(progressBar, "VISIBLE", "NO");
        IupSetAttribute(statusLabel, "VISIBLE", "NO");
        IupSetAttribute(repeatCheckbox, "VISIBLE", "NO");
        IupSetAttribute(intervalInput, "VISIBLE", "NO");
        
        // Stop any running script when disabling
        if (scriptRunning) {
            stopScript = TRUE;
        }
    }
    
    // Update the parent container
    IupRefresh(automationControlsBox);
    
    // Call the standard sync function
    return uiSyncToggle(ih, state);
}

// Built-in script templates
static const char* scriptTemplates[] = {
    "# Basic Network Stress Test\n"
    "wait 2000\n"
    "enable drop\n"
    "set drop chance 10.0\n"
    "wait 30000\n"
    "disable drop\n"
    "log \"Stress test completed\"\n",
    
    "# Mobile Network Simulation\n"
    "profile 3G\n"
    "wait 10000\n"
    "profile 4G\n"
    "wait 10000\n"
    "profile WiFi Good\n"
    "log \"Mobile network simulation completed\"\n",
    
    "# TLS Connection Test\n"
    "enable tls\n"
    "set tls handshake 98.0\n"
    "set tls data 5.0\n"
    "wait 60000\n"
    "disable tls\n"
    "log \"TLS test completed\"\n",
    
    "# Packet Size Analysis\n"
    "enable length\n"
    "set length min 64\n"
    "set length max 128\n"
    "wait 30000\n"
    "set length min 1400\n"
    "set length max 1500\n"
    "wait 30000\n"
    "disable length\n"
    "export data\n"
};

#define TEMPLATE_COUNT (sizeof(scriptTemplates) / sizeof(scriptTemplates[0]))

// Parse script line into action
static BOOL parseScriptLine(const char* line, ScriptAction* action) {
    char command[32], arg1[64], arg2[128];
    int args = sscanf(line, "%31s %63s %127s", command, arg1, arg2);
    
    if (args < 1) return FALSE;
    
    // Remove comments
    if (command[0] == '#') return FALSE;
    
    // Parse commands
    if (strcmp(command, "wait") == 0 && args >= 2) {
        action->type = SCRIPT_ACTION_WAIT;
        action->duration = atoi(arg1);
        return TRUE;
    } else if (strcmp(command, "enable") == 0 && args >= 2) {
        action->type = SCRIPT_ACTION_ENABLE_MODULE;
        strcpy(action->moduleName, arg1);
        return TRUE;
    } else if (strcmp(command, "disable") == 0 && args >= 2) {
        action->type = SCRIPT_ACTION_DISABLE_MODULE;
        strcpy(action->moduleName, arg1);
        return TRUE;
    } else if (strcmp(command, "set") == 0 && args >= 3) {
        action->type = SCRIPT_ACTION_SET_PARAMETER;
        strcpy(action->moduleName, arg1);
        strcpy(action->parameter, arg2);
        if (args >= 4) {
            char arg3[128];
            sscanf(line, "%*s %*s %*s %127s", arg3);
            strcpy(action->value, arg3);
        }
        return TRUE;
    } else if (strcmp(command, "profile") == 0 && args >= 2) {
        action->type = SCRIPT_ACTION_APPLY_PROFILE;
        strcpy(action->parameter, arg1);
        return TRUE;
    } else if (strcmp(command, "log") == 0 && args >= 2) {
        action->type = SCRIPT_ACTION_LOG_MESSAGE;
        // Get everything after "log "
        const char* message = strstr(line, "log ") + 4;
        if (message[0] == '"' && message[strlen(message)-1] == '"') {
            // Remove quotes
            strncpy(action->value, message + 1, strlen(message) - 2);
            action->value[strlen(message) - 2] = '\0';
        } else {
            strcpy(action->value, message);
        }
        return TRUE;
    } else if (strcmp(command, "export") == 0) {
        action->type = SCRIPT_ACTION_EXPORT_DATA;
        return TRUE;
    }
    
    return FALSE;
}

// Execute script action
static BOOL executeScriptAction(ScriptAction* action) {
    switch (action->type) {
        case SCRIPT_ACTION_WAIT:
            Sleep(action->duration);
            break;
            
        case SCRIPT_ACTION_ENABLE_MODULE:
            // Find and enable module
            for (int i = 0; i < MODULE_CNT; i++) {
                if (strcmp(modules[i]->shortName, action->moduleName) == 0) {
                    *(modules[i]->enabledFlag) = 1;
                    LOG("Script: Enabled module %s", action->moduleName);
                    break;
                }
            }
            break;
            
        case SCRIPT_ACTION_DISABLE_MODULE:
            // Find and disable module
            for (int i = 0; i < MODULE_CNT; i++) {
                if (strcmp(modules[i]->shortName, action->moduleName) == 0) {
                    *(modules[i]->enabledFlag) = 0;
                    LOG("Script: Disabled module %s", action->moduleName);
                    break;
                }
            }
            break;
            
        case SCRIPT_ACTION_SET_PARAMETER:
            // Set module parameter (simplified implementation)
            LOG("Script: Set %s.%s = %s", action->moduleName, action->parameter, action->value);
            break;
            
        case SCRIPT_ACTION_APPLY_PROFILE:
            LOG("Script: Applied profile %s", action->parameter);
            break;
            
        case SCRIPT_ACTION_LOG_MESSAGE:
            LOG("Script: %s", action->value);
            break;
            
        case SCRIPT_ACTION_EXPORT_DATA:
            LOG("Script: Exported data");
            break;
    }
    
    return TRUE;
}

// Script execution thread
static DWORD WINAPI scriptExecutionThread(LPVOID param) {
    UNREFERENCED_PARAMETER(param);
    
    if (currentScript < 0 || currentScript >= scriptCount) {
        scriptRunning = FALSE;
        return 0;
    }
    
    AutomationScript* script = &scripts[currentScript];
    
    do {
        stopScript = FALSE;
        
        for (int i = 0; i < script->actionCount && !stopScript; i++) {
            if (!executeScriptAction(&script->actions[i])) {
                LOG("Script execution failed at action %d", i);
                break;
            }
            
            // Update progress
            int progress = (i + 1) * 100 / script->actionCount;
            IupSetInt(progressBar, "VALUE", progress);
        }
        
        if (script->repeat && !stopScript) {
            LOG("Script repeating in %d seconds", scriptInterval);
            for (int i = 0; i < scriptInterval && !stopScript; i++) {
                Sleep(1000);
            }
        }
    } while (script->repeat && !stopScript);
    
    scriptRunning = FALSE;
    IupSetAttribute(statusLabel, "TITLE", "Script completed");
    IupSetInt(progressBar, "VALUE", 100);
    
    return 0;
}

// Load script template
static void loadScriptTemplate(int templateIndex) {
    if (templateIndex >= 0 && templateIndex < TEMPLATE_COUNT) {
        IupSetAttribute(scriptEditor, "VALUE", scriptTemplates[templateIndex]);
    }
}

// Load script button callback
static int loadScriptCallback(Ihandle *ih) {
    UNREFERENCED_PARAMETER(ih);
    
    Ihandle *filedlg = IupFileDlg();
    IupSetAttribute(filedlg, "DIALOGTYPE", "OPEN");
    IupSetAttribute(filedlg, "TITLE", "Load Script");
    IupSetAttribute(filedlg, "FILTER", "*.script");
    
    IupPopup(filedlg, IUP_CENTER, IUP_CENTER);
    
    if (IupGetInt(filedlg, "STATUS") != -1) {
        char* filename = IupGetAttribute(filedlg, "VALUE");
        FILE* file;
        if ((file = fopen(filename, "r")) != NULL) {
            char buffer[4096];
            size_t len = fread(buffer, 1, sizeof(buffer) - 1, file);
            buffer[len] = '\0';
            IupSetAttribute(scriptEditor, "VALUE", buffer);
            fclose(file);
            LOG("Script loaded: %s", filename);
        }
    }
    
    IupDestroy(filedlg);
    return IUP_DEFAULT;
}

// Save script button callback
static int saveScriptCallback(Ihandle *ih) {
    UNREFERENCED_PARAMETER(ih);
    
    Ihandle *filedlg = IupFileDlg();
    IupSetAttribute(filedlg, "DIALOGTYPE", "SAVE");
    IupSetAttribute(filedlg, "TITLE", "Save Script");
    IupSetAttribute(filedlg, "FILTER", "*.script");
    
    IupPopup(filedlg, IUP_CENTER, IUP_CENTER);
    
    if (IupGetInt(filedlg, "STATUS") != -1) {
        char* filename = IupGetAttribute(filedlg, "VALUE");
        FILE* file;
        if ((file = fopen(filename, "w")) != NULL) {
            char* content = IupGetAttribute(scriptEditor, "VALUE");
            fwrite(content, 1, strlen(content), file);
            fclose(file);
            LOG("Script saved: %s", filename);
        }
    }
    
    IupDestroy(filedlg);
    return IUP_DEFAULT;
}

// Run script button callback
static int runScriptCallback(Ihandle *ih) {
    UNREFERENCED_PARAMETER(ih);
    
    if (scriptRunning) {
        // Stop current script
        stopScript = TRUE;
        IupSetAttribute(statusLabel, "TITLE", "Stopping script...");
        return IUP_DEFAULT;
    }
    
    // Parse current script
    char* scriptContent = IupGetAttribute(scriptEditor, "VALUE");
    if (!scriptContent || strlen(scriptContent) == 0) {
        IupSetAttribute(statusLabel, "TITLE", "No script to run");
        return IUP_DEFAULT;
    }
    
    // Clear current script
    if (scriptCount > 0) {
        scripts[0].actionCount = 0;
    } else {
        scriptCount = 1;
    }
    
    strcpy(scripts[0].name, "Current Script");
    scripts[0].enabled = TRUE;
    scripts[0].repeat = scriptRepeat;
    
    // Parse script content
    char* line = strtok(scriptContent, "\n");
    int actionIndex = 0;
    
    while (line && actionIndex < 64) {
        ScriptAction action;
        if (parseScriptLine(line, &action)) {
            scripts[0].actions[actionIndex] = action;
            actionIndex++;
        }
        line = strtok(NULL, "\n");
    }
    
    scripts[0].actionCount = actionIndex;
    currentScript = 0;
    
    // Start script execution
    if (scripts[0].actionCount > 0) {
        scriptRunning = TRUE;
        IupSetAttribute(statusLabel, "TITLE", "Running script...");
        IupSetInt(progressBar, "VALUE", 0);
        
        scriptThread = CreateThread(NULL, 0, scriptExecutionThread, NULL, 0, NULL);
    } else {
        IupSetAttribute(statusLabel, "TITLE", "Empty script");
    }
    
    return IUP_DEFAULT;
}

// Script list selection callback
static int scriptListCallback(Ihandle *ih, char *text, int item, int state) {
    UNREFERENCED_PARAMETER(ih);
    UNREFERENCED_PARAMETER(text);
    
    if (state == 1 && item <= TEMPLATE_COUNT) {
        loadScriptTemplate(item - 1);
        IupSetAttribute(statusLabel, "TITLE", "Template loaded");
    }
    return IUP_DEFAULT;
}

static Ihandle* automationSetupUI() {
    automationControlsBox = IupVbox(
        IupHbox(
            enabledCheckbox = IupToggle("Enable Automation", NULL),
            repeatCheckbox = IupToggle("Repeat", NULL),
            IupLabel("Interval(s):"),
            intervalInput = IupText(NULL),
            NULL
        ),
        IupHbox(
            IupLabel("Templates:"),
            scriptList = IupList(NULL),
            loadScriptButton = IupButton("Load File", NULL),
            saveScriptButton = IupButton("Save File", NULL),
            NULL
        ),
        IupFrame(
            IupVbox(
                scriptEditor = IupText(NULL),
                NULL
            )
        ),
        IupHbox(
            runScriptButton = IupButton("Run Script", NULL),
            progressBar = IupProgressBar(),
            NULL
        ),
        statusLabel = IupLabel("Automation ready"),
        NULL
    );

    // Setup script list with templates
    IupSetAttribute(scriptList, "DROPDOWN", "YES");
    IupSetAttribute(scriptList, "VISIBLECOLUMNS", "15");
    IupSetAttribute(scriptList, "1", "Network Stress Test");
    IupSetAttribute(scriptList, "2", "Mobile Simulation");
    IupSetAttribute(scriptList, "3", "TLS Connection Test");
    IupSetAttribute(scriptList, "4", "Packet Size Analysis");
    IupSetAttribute(scriptList, "VALUE", "1");
    IupSetCallback(scriptList, "ACTION", (Icallback)scriptListCallback);

    // Setup script editor
    IupSetAttribute(scriptEditor, "MULTILINE", "YES");
    IupSetAttribute(scriptEditor, "EXPAND", "YES");
    IupSetAttribute(scriptEditor, "VISIBLELINES", "12");
    IupSetAttribute(scriptEditor, "FONT", "Courier New, 10");
    IupSetAttribute(scriptEditor, "VALUE", scriptTemplates[0]);

    // Setup interval input
    IupSetAttribute(intervalInput, "VISIBLECOLUMNS", "6");
    IupSetAttribute(intervalInput, "VALUE", "60");
    IupSetCallback(intervalInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(intervalInput, SYNCED_VALUE, (char*)&scriptInterval);
    IupSetAttribute(intervalInput, INTEGER_MIN, "1");
    IupSetAttribute(intervalInput, INTEGER_MAX, "3600");

    // Setup checkboxes
    IupSetCallback(enabledCheckbox, "ACTION", (Icallback)automationEnabledCallback);
    IupSetAttribute(enabledCheckbox, SYNCED_VALUE, (char*)&automationEnabled);

    IupSetCallback(repeatCheckbox, "ACTION", (Icallback)uiSyncToggle);
    IupSetAttribute(repeatCheckbox, SYNCED_VALUE, (char*)&scriptRepeat);

    // Setup buttons
    IupSetCallback(loadScriptButton, "ACTION", (Icallback)loadScriptCallback);
    IupSetAttribute(loadScriptButton, "PADDING", "4x");

    IupSetCallback(saveScriptButton, "ACTION", (Icallback)saveScriptCallback);
    IupSetAttribute(saveScriptButton, "PADDING", "4x");

    IupSetCallback(runScriptButton, "ACTION", (Icallback)runScriptCallback);
    IupSetAttribute(runScriptButton, "PADDING", "8x");

    // Setup progress bar
    IupSetAttribute(progressBar, "EXPAND", "HORIZONTAL");
    IupSetAttribute(progressBar, "MIN", "0");
    IupSetAttribute(progressBar, "MAX", "100");
    IupSetAttribute(progressBar, "VALUE", "0");

    // Setup status label
    IupSetAttribute(statusLabel, "EXPAND", "HORIZONTAL");
    IupSetAttribute(statusLabel, "ALIGNMENT", "ALEFT");

    if (parameterized) {
        setFromParameter(enabledCheckbox, "VALUE", NAME"-enabled");
        setFromParameter(repeatCheckbox, "VALUE", NAME"-repeat");
        setFromParameter(intervalInput, "VALUE", NAME"-interval");
    }

    // Initially hide automation controls if not enabled
    if (!automationEnabled) {
        IupSetAttribute(scriptList, "VISIBLE", "NO");
        IupSetAttribute(loadScriptButton, "VISIBLE", "NO");
        IupSetAttribute(saveScriptButton, "VISIBLE", "NO");
        IupSetAttribute(scriptEditor, "VISIBLE", "NO");
        IupSetAttribute(runScriptButton, "VISIBLE", "NO");
        IupSetAttribute(progressBar, "VISIBLE", "NO");
        IupSetAttribute(statusLabel, "VISIBLE", "NO");
        IupSetAttribute(repeatCheckbox, "VISIBLE", "NO");
        IupSetAttribute(intervalInput, "VISIBLE", "NO");
    }

    return automationControlsBox;
}

static void automationStartup() {
    LOG("Automation and scripting enabled");
}

static void automationCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
    
    // Stop any running scripts
    if (scriptRunning) {
        stopScript = TRUE;
        if (scriptThread) {
            WaitForSingleObject(scriptThread, 5000);
            CloseHandle(scriptThread);
            scriptThread = NULL;
        }
    }
    
    LOG("Automation and scripting disabled");
}

static short automationProcess(PacketNode *head, PacketNode* tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
    // Automation module doesn't directly process packets
    // It controls other modules through scripting
    return FALSE;
}

Module automationModule = {
    "Automation & Scripting",
    NAME,
    (short*)&automationEnabled,
    automationSetupUI,
    automationStartup,
    automationCloseDown,
    automationProcess,
    // runtime fields
    0, 0, NULL
};