/**
 * @file utilities.cpp
 * @brief Implementation of global enablements, utility functions, and configuration mappings 
 *        used throughout the indoor localization system.
 */

#include "utilities.h"
#include "platform.h"

// ====================== System Setup ======================

// == System Mode ==
SystemMode         SystemSetup::currentSystemMode        = MODES_NUM;
SystemScannerMode  SystemSetup::currentSystemScannerMode = SYSTEM_SCANNER_MODES_NUM;
SystemBootMode     SystemSetup::currentSystemBootMode    = SYSTEM_BOOT_MODES_NUM; 

// == Features ==
bool SystemSetup::enableBackup          = false;
bool SystemSetup::enableRestore         = false;
bool SystemSetup::enableValidationPhase = false;

// == Logging ==
LogLevel SystemSetup::logLevel   = LOG_LEVEL_ERROR;


// ======================   Globals    ======================

Label  currentLabel = LABELS_COUNT;
bool   shouldAbort  = false;
bool   reconfigure  = true;

bool   reuseFromMemory[LABELS_COUNT] = {0};
bool   validForPredection[LABELS_COUNT];
double tofAccuracy[LABELS_COUNT] = {0};
double rssiAccuracy[LABELS_COUNT] = {0};

ScannerFlag BufferedData::scanner  = NONE;
int         BufferedData::lastN    = 0;

std::vector<RSSIData>  rssiDataSet = {};
std::vector<TOFData>   tofDataSet  = {};

double  accumulatedRSSIs[NUMBER_OF_ANCHORS];
double  accumlatedTOFS[NUMBER_OF_RESPONDERS];
uint8_t responderMacs[NUMBER_OF_RESPONDERS][TOF_NUMBER_OF_MAC_BYTES];


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

const std::string tofSSIDs[NUMBER_OF_ANCHORS] = {
    "RESP1",
    "RESP2",
    "RESP3",
    "RESP4"
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

const std::string systemModes[MODES_NUM] = {
    "MODE_SYSTEM_BOOT",
    "MODE_SCANNING_SESSION",
    "MODE_PREDICTION_SESSION",
    "MODE_FULL_SESSION"
};

const std::string systemScannerModes[SYSTEM_SCANNER_MODES_NUM] {
    "STATIC_RSSI",
    "TOF",
    "STATIC_RSSI_TOF"
};

const std::string systemBootModes[SYSTEM_BOOT_MODES_NUM] {
    "MODE_TOF_DIAGNOSTIC",
    "MODE_COLLECT_TOF_RESPONDERS_MAC",
    "MODE_RSSI_DIAGNOSTIC",
    "MODE_SD_CARD_TEST"
};

const std::string systemStates[SYSTEM_SCANNER_MODES_NUM] = {
    "STATIC_RSSI",
    "TOF",
    "STATIC_RSSI_TOF"
};


// ====================== Utility Functions ======================

int applyEMA(int prevRSSI, int newRSSI) {
    if (prevRSSI == RSSI_DEFAULT_VALUE) return newRSSI;
    return (int)(ALPHA * prevRSSI + (1.0 - ALPHA) * newRSSI);
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

void resetRssiBuffer() {
    for(int i = 0; i < NUMBER_OF_ANCHORS; i++) {
        accumulatedRSSIs[i] = RSSI_DEFAULT_VALUE;
    }
}

void resetTOFScanBuffer() {
    for(int i = 0; i < NUMBER_OF_RESPONDERS; i++) {
        accumulatedTOFs[i] = TOF_DEFAULT_DISTANCE_CM;
    }
}

void setValidForPredection() {
    for (int i = 0; i < LABELS_COUNT; i++) {
        validForPredection[i] = true;
    }
}

float getAccuracy() {
    if (BufferedData::scanner == STATICRSSI) return rssiAccuracy[currentLabel];
    if (BufferedData::scanner == TOF_)       return tofAccuracy[currentLabel];
    if (BufferedData::scanner == BOTH)       return (rssiAccuracy[currentLabel] + tofAccuracy[currentLabel]) / 2.0;
    return 0.0;
}
