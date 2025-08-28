// network condition profiles module
#include <stdlib.h>
#include <Windows.h>
#include "iup.h"
#include "common.h"
#define NAME "profiles"

// Network profile definitions
typedef struct {
    const char* name;
    int lag_min;        // ms
    int lag_max;        // ms
    int loss_rate;      // 0-10000 (0-100%)
    int bandwidth;      // kbps
    int jitter;         // ms
    int duplicate_rate; // 0-10000 (0-100%)
} NetworkProfile;

// Predefined network profiles
static NetworkProfile profiles[] = {
    {"2G (GPRS)",      400,  800,  500,   50,   100,  100},  // 2G: 400-800ms lag, 5% loss, 50kbps
    {"2.5G (EDGE)",    200,  500,  300,  200,    80,   50},  // 2.5G: 200-500ms lag, 3% loss, 200kbps
    {"3G",             100,  300,  200, 2000,    50,   30},  // 3G: 100-300ms lag, 2% loss, 2Mbps
    {"3.5G (HSPA)",     50,  150,  100, 8000,    30,   20},  // 3.5G: 50-150ms lag, 1% loss, 8Mbps
    {"4G (LTE)",        20,   80,   50,50000,    20,   10},  // 4G: 20-80ms lag, 0.5% loss, 50Mbps
    {"5G",              10,   40,   20,100000,   10,    5},  // 5G: 10-40ms lag, 0.2% loss, 100Mbps
    {"WiFi (Good)",     10,   50,   30,25000,    15,   10},  // WiFi Good: 10-50ms lag, 0.3% loss, 25Mbps
    {"WiFi (Poor)",     50,  200,  800, 2000,    80,  100},  // WiFi Poor: 50-200ms lag, 8% loss, 2Mbps
    {"Satellite",      500,  800,  200,20000,   100,   50},  // Satellite: 500-800ms lag, 2% loss, 20Mbps
    {"Dial-up",       1000, 2000, 1000,   56,   200,  200},  // Dial-up: 1-2s lag, 10% loss, 56kbps
    {"Custom",            0,    0,    0,    0,     0,    0}   // Custom profile
};

#define PROFILE_COUNT (sizeof(profiles) / sizeof(profiles[0]))

static Ihandle *profileList, *applyButton, *detailsLabel;
static Ihandle *customLagInput, *customLossInput, *customBandwidthInput;

static volatile short profilesEnabled = 0;
static volatile short selectedProfile = 0;
static volatile short customLag = 100, customLoss = 100, customBandwidth = 1000;

// Apply profile settings to other modules via UI
static void applyProfile(int profileIndex) {
    if (profileIndex < 0 || profileIndex >= PROFILE_COUNT) return;
    
    NetworkProfile* profile = &profiles[profileIndex];
    
    LOG("Applying network profile: %s", profile->name);
    
    // Instead of direct variable access, we'll need to update UI elements
    // and let the sync mechanisms handle the updates
    char valueStr[32];
    
    if (profileIndex == PROFILE_COUNT - 1) { // Custom profile
        sprintf(valueStr, "%d", customLag);
        LOG("Custom profile: lag=%dms, loss=%.1f%%, bandwidth=%dkbps", 
            customLag, customLoss/100.0, customBandwidth);
    } else {
        sprintf(valueStr, "%d", (profile->lag_min + profile->lag_max) / 2);
        LOG("Profile applied: %s - lag=%dms, loss=%.1f%%, bandwidth=%dkbps", 
            profile->name, (profile->lag_min + profile->lag_max) / 2, 
            profile->loss_rate/100.0, profile->bandwidth);
    }
    
    // Note: In the real implementation, we would need access to other module's UI elements
    // to properly set their values. This would require architectural changes.
    // For now, this is a placeholder that demonstrates the concept.
}

// Update details display
static void updateProfileDetails(int profileIndex) {
    char details[512];
    
    if (profileIndex < 0 || profileIndex >= PROFILE_COUNT) return;
    
    NetworkProfile* profile = &profiles[profileIndex];
    
    if (profileIndex == PROFILE_COUNT - 1) { // Custom profile
        snprintf(details, sizeof(details),
            "Custom Profile:\nLag: %dms | Loss: %.1f%% | Bandwidth: %dkbps",
            customLag, customLoss/100.0, customBandwidth);
    } else {
        snprintf(details, sizeof(details),
            "%s:\nLag: %d-%dms | Loss: %.1f%% | Bandwidth: %dkbps | Jitter: %dms",
            profile->name, profile->lag_min, profile->lag_max,
            profile->loss_rate/100.0, profile->bandwidth, profile->jitter);
    }
    
    IupStoreAttribute(detailsLabel, "TITLE", details);
}

// Profile selection callback
static int profileSelectCallback(Ihandle *ih, char *text, int item, int state) {
    UNREFERENCED_PARAMETER(ih);
    UNREFERENCED_PARAMETER(text);
    
    if (state == 1) {
        selectedProfile = item - 1; // Convert from 1-based to 0-based
        updateProfileDetails(selectedProfile);
        
        // Show/hide custom controls
        if (selectedProfile == PROFILE_COUNT - 1) {
            IupSetAttribute(customLagInput, "VISIBLE", "YES");
            IupSetAttribute(customLossInput, "VISIBLE", "YES");
            IupSetAttribute(customBandwidthInput, "VISIBLE", "YES");
        } else {
            IupSetAttribute(customLagInput, "VISIBLE", "NO");
            IupSetAttribute(customLossInput, "VISIBLE", "NO");
            IupSetAttribute(customBandwidthInput, "VISIBLE", "NO");
        }
    }
    return IUP_DEFAULT;
}

// Apply button callback
static int applyProfileCallback(Ihandle *ih) {
    UNREFERENCED_PARAMETER(ih);
    
    applyProfile(selectedProfile);
    return IUP_DEFAULT;
}

static Ihandle* profilesSetupUI() {
    Ihandle *profilesControlsBox = IupVbox(
        IupHbox(
            IupLabel("Network Profile:"),
            profileList = IupList(NULL),
            applyButton = IupButton("Apply", NULL),
            NULL
        ),
        detailsLabel = IupLabel("Select a profile to see details..."),
        IupHbox(
            IupLabel("Custom - Lag(ms):"),
            customLagInput = IupText(NULL),
            IupLabel("Loss(%):"),
            customLossInput = IupText(NULL),
            IupLabel("BW(kbps):"),
            customBandwidthInput = IupText(NULL),
            NULL
        ),
        NULL
    );

    // Setup profile list
    IupSetAttribute(profileList, "DROPDOWN", "YES");
    IupSetAttribute(profileList, "VISIBLECOLUMNS", "15");
    
    for (int i = 0; i < PROFILE_COUNT; i++) {
        char indexStr[8];
        sprintf(indexStr, "%d", i + 1);
        IupStoreAttribute(profileList, indexStr, profiles[i].name);
    }
    
    IupSetAttribute(profileList, "VALUE", "1");
    IupSetCallback(profileList, "ACTION", (Icallback)profileSelectCallback);

    // Setup apply button
    IupSetCallback(applyButton, "ACTION", (Icallback)applyProfileCallback);
    IupSetAttribute(applyButton, "PADDING", "4x");

    // Setup custom inputs
    IupSetAttribute(customLagInput, "VISIBLECOLUMNS", "6");
    IupSetAttribute(customLagInput, "VALUE", "100");
    IupSetAttribute(customLagInput, "VISIBLE", "NO");
    IupSetCallback(customLagInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(customLagInput, SYNCED_VALUE, (char*)&customLag);
    IupSetAttribute(customLagInput, INTEGER_MIN, "0");
    IupSetAttribute(customLagInput, INTEGER_MAX, "5000");

    IupSetAttribute(customLossInput, "VISIBLECOLUMNS", "6");
    IupSetAttribute(customLossInput, "VALUE", "1.0");
    IupSetAttribute(customLossInput, "VISIBLE", "NO");
    IupSetCallback(customLossInput, "VALUECHANGED_CB", uiSyncChance);
    IupSetAttribute(customLossInput, SYNCED_VALUE, (char*)&customLoss);

    IupSetAttribute(customBandwidthInput, "VISIBLECOLUMNS", "8");
    IupSetAttribute(customBandwidthInput, "VALUE", "1000");
    IupSetAttribute(customBandwidthInput, "VISIBLE", "NO");
    IupSetCallback(customBandwidthInput, "VALUECHANGED_CB", uiSyncInteger);
    IupSetAttribute(customBandwidthInput, SYNCED_VALUE, (char*)&customBandwidth);
    IupSetAttribute(customBandwidthInput, INTEGER_MIN, "1");
    IupSetAttribute(customBandwidthInput, INTEGER_MAX, "1000000");

    // Setup details label
    IupSetAttribute(detailsLabel, "EXPAND", "HORIZONTAL");
    IupSetAttribute(detailsLabel, "ALIGNMENT", "ALEFT");
    IupSetAttribute(detailsLabel, "PADDING", "4x4");

    // Initialize with first profile
    updateProfileDetails(0);

    if (parameterized) {
        setFromParameter(profileList, "VALUE", NAME"-profile");
        setFromParameter(customLagInput, "VALUE", NAME"-lag");
        setFromParameter(customLossInput, "VALUE", NAME"-loss");
        setFromParameter(customBandwidthInput, "VALUE", NAME"-bandwidth");
    }

    return profilesControlsBox;
}

static void profilesStartup() {
    LOG("network profiles enabled");
}

static void profilesCloseDown(PacketNode *head, PacketNode *tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
    LOG("network profiles disabled");
}

static short profilesProcess(PacketNode *head, PacketNode* tail) {
    UNREFERENCED_PARAMETER(head);
    UNREFERENCED_PARAMETER(tail);
    // Profiles module doesn't directly process packets
    // It configures other modules instead
    return FALSE;
}

Module profilesModule = {
    "Network Profiles",
    NAME,
    (short*)&profilesEnabled,
    profilesSetupUI,
    profilesStartup,
    profilesCloseDown,
    profilesProcess,
    // runtime fields
    0, 0, NULL
};