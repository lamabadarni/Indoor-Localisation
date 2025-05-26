/**
 * @file utilities.cpp
 * @brief Implementation of global enablements, utility functions, and configuration mappings 
 *        used throughout the indoor localization system.
 */

#include "utilities.h"
#include "platform.h"

// ====================== System Setup ======================

// == System Mode ==
SystemSetup::currentSystemMode        = MODES_NUM;
SystemSetup::currentSystemBootMode    = SYSTEM_BOOT_MODES_NUM;
SystemSetup::currentSystemScannerMode = SYSTEM_SCANNER_MODES_NUM;

// == Features ==
SystemSetup::chooseLabelsToScan         = false;
SystemSetup::forceSDCardBackup          = false;
SystemSetup::validateWhileScanningPhase = false;

// == Logging ==
SystemSetup::printInfoLogs    = true;
SystemSetup::printWarningLogs = true;
SystemSetup::printDebugLogs   = false;


// ======================   Globals    ======================

bool    reuseFromSD[LABELS_COUNT]  = {false};
double  accuracy                   = -1;
bool    shouldAbort                = false;
// ======================  CONST Globals    ======================

const std::string anchorSSIDs[NUMBER_OF_ANCHORS] = {
    "234/236",
    "236",
    "231/236",
    "231", 
    "Kitchen",
    "Entrance",
    "Lobby",
    "Balcony",
    "Offices"
};

const std::string labels[LABELS_COUNT] = {
    "NEAR_ROOM_234",
    "BETWEEN_ROOMS_234_236",
    "ROOM_236",
    "NEAR_ROOM_232",
    "ROOM_231",
    "BETWEEN_ROOMS_231_236",
    "NEAR_BATHROOM",
    "NEAR_KITCHEN",
    "KITCHEN",
    "NEAR_ROOM_230",
    "MAIN_ENTRANCE",
    "LOBBY",
    "PRINTER",
    "OFFICES_HALL",
    "MAIN_EXIT",
    "BALCONY_ENTRANCE"
};

const std::string systemStates[SYSTEM_SCANNER_MODES_NUM] = {
    "STATIC_RSSI",
    "TOF",
    "STATIC_RSSI_TOF"
};

const std::string systemModes[MODES_NUM] = {
    "MODE_SYSTEM_BOOT",
    "MODE_SCANNING_SESSION",
    "MODE_PREDICTION_SESSION",
    "MODE_FULL_SESSION"
};

const std::string systemBootModes[SYSTEM_BOOT_MODES_NUM] {
    "MODE_TOF_DIAGNOSTIC",
    "MODE_TOF_COMMUNICATION_TEST",
    "MODE_COLLECT_TOF_RESPONDERS_MAC",
    "MODE_RSSI_DIAGNOSTIC",
    "MODE_SD_CARD_TEST"
};

const std::string systemScannerModes[SYSTEM_SCANNER_MODES_NUM] {
    "STATIC_RSSI",
    "TOF",
    "STATIC_RSSI_TOF"
};

// ====================== Utility Functions ======================

int applyEMA(int prevRSSI, int newRSSI) {
    if (prevRSSI == RSSI_DEFAULT_VALUE) return newRSSI;
    return (int)(ALPHA * prevRSSI + (1.0f - ALPHA) * newRSSI);
}

int getChoosenLabelsCount() {
    int ctr = 0;
    for(int i =  0; i < LABELS_COUNT; i++) {
        if(labels[i]) ctr++;
    }

    return ctr;
}

char readCharFromUser() {
    char input[8];
    fgets(input, sizeof(input), stdin);
    return input[0];
}

int readIntFromUser() {
    char input[16];
    fgets(input, sizeof(input), stdin);
    return atoi(input);
}

char getCharFromUserWithTimeout(int timeoutMs) {
    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;

    int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (ret > 0 && FD_ISSET(STDIN_FILENO, &fds)) {
        char input[8] = {0};
        fgets(input, sizeof(input), stdin);
        return input[0];
    }

    return '\0';  // timeout or no input
}