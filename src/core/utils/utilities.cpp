/**
 * @file utilities.cpp
 * @brief Implementation of global enablements, utility functions, and configuration mappings 
 *        used throughout the indoor localization system.
 */

#include "utilities.h"
#include "platform.h"

// ====================== System Setup ======================

// == System Mode ==
SystemMode            SystemSetup::currentSystemMode           = MODES_NUM;
SystemScannerMode     SystemSetup::currentSystemScannerMode    = SYSTEM_SCANNER_MODES_NUM;
SystemPredictionMode  SystemSetup::currentSystemPredictionMode = SYSTEM_PREDICTION_NODES_NUM;
SystemBootMode        SystemSetup::currentSystemBootMode       = SYSTEM_BOOT_MODES_NUM; 

// == Features ==
bool SystemSetup::enableValidationPhase = false;

// == Logging ==
LogLevel SystemSetup::logLevel   = LOG_LEVEL_ERROR;

SystemScannerMode SaveBufferedData::scanner   = SYSTEM_SCANNER_MODES_NUM;
SystemScannerMode DeleteBufferedData::scanner = SYSTEM_SCANNER_MODES_NUM;
int               SaveBufferedData::lastN     = 0;
int               DeleteBufferedData::lastN   = 0;

bool DataLoaded::Dynamic = false;
bool DataLoaded::Static  = false;
bool DataLoaded::TOF     = false;

// ======================   Globals    ======================

Label  currentLabel = LABELS_COUNT;
bool   shouldAbort  = false;

bool   validForPredection[LABELS_COUNT]  = {0};
double tofAccuracy[LABELS_COUNT]         = {0};
double staticRSSIAccuracy[LABELS_COUNT]  = {0};
double dynamicRSSIAccuracy[LABELS_COUNT] = {0};

std::vector<StaticRSSIData>  staticRSSIDataSet  = {};
std::vector<DynamicMacData>  dynamicMacDataSet  = {};
std::vector<DynamicRSSIData> dynamicRSSIDataSet = {};
std::vector<TOFData>         tofDataSet         = {};
std::vector<std::string>     log_buffer;

double  accumulatedStaticRSSIs[NUMBER_OF_ANCHORS];
double  accumulatedTOFs[NUMBER_OF_RESPONDERS];
double  accumulatedDynamicRSSIs[NUMBER_OF_DYNAMIC_APS];
uint8_t accumulatedMacAddresses[NUMBER_OF_DYNAMIC_APS][MAC_ADDRESS_SIZE];
uint8_t responderMacs[NUMBER_OF_RESPONDERS][MAC_ADDRESS_SIZE];

FILE* logFile = NULL;


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

const std::string tofSSIDs[NUMBER_OF_RESPONDERS] = {
    "RESP1",
    "RESP2",
    "RESP3",
    "RESP4"
};

const std::string labels[LABELS_COUNT] = {
    "ROOM_236",
    "BETWEEN_ROOMS_234_236",
    "BETWEEN_ROOM_232_236",
    "ROOM_231",
    "BETWEEN_ROOMS_231_236",
    "NEAR_KITCHEN_BATHROOM",
    "KITCHEN",
    "MAIN_ENTRANCE",
    "LOBBY",
    "PRINTER",
    "OFFICES_HALL_1",
    "OFFICES_HALL_2",
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
    "DYNAMIC_RSSI",
    "TOF"
};

const std::string systemPredictionModes[SYSTEM_PREDICTION_NODES_NUM] {
    "STATIC_RSSI", 
    "DYNAMIC_RSSI",
    "TOF",
    "STATIC_RSSI_DYNAMIC_RSSI",
    "STATIC_RSSI_TOF",
    "DYNAMIC_RSSI_TOF",
    "STATIC_RSSI_DYNAMIC_RSSI_TOF",
};

const std::string systemBootModes[SYSTEM_BOOT_MODES_NUM] {
    "MODE_TOF_DIAGNOSTIC",
    "MODE_ANCHORS_RSSI_DIAGNOSTIC",
    "MODE_APS_RSSI_DIAGNOSTIC",
    "MODE_COLLECT_TOF_RESPONDERS_MAC",
    "MODE_INITIATOR_RESPONDER_TEST",
    "MODE_RESTORE_BACKUP_DATA_TEST"
};

const std::string coverages[COVERAGE_STATES_NUM] {
    "GOOD COVERAGE",
    "WEAK COVERAGE",
    "BAD COVERAGE"
};

UI const systemUI = UI::SERIAL;

// ====================== Utility Functions ======================

int applyEMA(int prevRSSI, int newRSSI) {
    if (prevRSSI == RSSI_DEFAULT_VALUE) return newRSSI;
    return (int)(ALPHA * prevRSSI + (1.0 - ALPHA) * newRSSI);
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
    

void resetStaticRssiBuffer() {
    for(int i = 0; i < NUMBER_OF_ANCHORS; i++) {
        accumulatedStaticRSSIs[i] = RSSI_DEFAULT_VALUE;
    }
}

void resetTOFScanBuffer() {
    for(int i = 0; i < NUMBER_OF_RESPONDERS; i++) {
        accumulatedTOFs[i] = TOF_DEFAULT_DISTANCE_CM;
    }
}

void resetDynamicRssiBuffer() {
    for(int i = 0; i < NUMBER_OF_DYNAMIC_APS; i++) {
        accumulatedDynamicRSSIs[i] = RSSI_DEFAULT_VALUE;
    }
}

void setValidForPredection() {
    for (int i = 0; i < LABELS_COUNT; i++) {
        validForPredection[i] = true;
    }
}

float getAccuracyForValidation() {
    switch ( SystemSetup::currentSystemPredictionMode ) {
        case SystemPredictionMode::STATICRSSI :
            return staticRSSIAccuracy[currentLabel];
        case SystemPredictionMode::DYNAMICRSSI :
            return dynamicRSSIAccuracy[currentLabel];
        case SystemPredictionMode::TOfF :
            return tofAccuracy[currentLabel];
        default:
            break;
    } 
    return 0.0;
}

bool isStaticRSSIActiveForPrediction() {
    return SystemSetup::currentSystemPredictionMode == STATICRSSI || 
           SystemSetup::currentSystemPredictionMode == STATIC_RSSI_DYNAMIC_RSSI || 
           SystemSetup::currentSystemPredictionMode == STATIC_RSSI_TOF ||
           SystemSetup::currentSystemPredictionMode == STATIC_RSSI_DYNAMIC_RSSI_TOF;
}

bool isDynamicRSSIActiveForPrediction() {
    return SystemSetup::currentSystemPredictionMode == DYNAMICRSSI || 
           SystemSetup::currentSystemPredictionMode == STATIC_RSSI_DYNAMIC_RSSI || 
           SystemSetup::currentSystemPredictionMode == DYNAMIC_RSSI_TOF ||
           SystemSetup::currentSystemPredictionMode == STATIC_RSSI_DYNAMIC_RSSI_TOF;
}

bool isTOFActiveForPrediction() {
    return SystemSetup::currentSystemPredictionMode == TOfF || 
           SystemSetup::currentSystemPredictionMode == STATIC_RSSI_TOF || 
           SystemSetup::currentSystemPredictionMode == DYNAMIC_RSSI_TOF ||
           SystemSetup::currentSystemPredictionMode == STATIC_RSSI_DYNAMIC_RSSI_TOF;
}

bool isDataLoadedForScan() {
    switch (SystemSetup::currentSystemScannerMode) {
    case TOF:
        return DataLoaded::TOF;
    case DYNAMIC_RSSI:
        return DataLoaded::Dynamic;
    case STATIC_RSSI:
        return DataLoaded::Static;
    default:
        break;
    }
    return false;
}

bool isDataLoadedForPrediction() {
    bool loaded = false;
    if(isDynamicRSSIActiveForPrediction()) {
        loaded = DataLoaded::Dynamic;
    }
    if(isStaticRSSIActiveForPrediction()) {
        loaded = DataLoaded::Static;
    }
    if(isTOFActiveForPrediction()) {
        loaded = DataLoaded::TOF;
    }

    return loaded;
}

std::vector<std::string> arrayToVector(const std::string arr[], int size) {
    std::vector<std::string> vec;
    for (int i = 0; i < size; ++i) {
        vec.push_back(arr[i]);
    }
    return vec;
}
