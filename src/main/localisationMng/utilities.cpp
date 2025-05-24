/**
 * @file utilities.cpp
 * @brief Implementation of global enablements, utility functions, and configuration mappings 
 *        used throughout the indoor localization system.
 */

#include "utillities.h"

// ====================== Enablements ======================

bool Enablements::enable_training_model_on_host_machine = false;
bool Enablements::enable_SD_Card_backup                 = false;
bool Enablements::run_validation_phase                  = false;
bool Enablements::verify_responder_mac_mapping          = false;
bool Enablements::verify_rssi_anchor_mapping            = false;

SystemState currentSystemState                          = OFFLINE;
SystemMode  currentSystemMode                           = MODE_FULL_SESSION;

bool reuseFromSD[NUMBER_OF_LABELS] = {false};

// ====================== Label Conversion ======================

const char* locationToString(int loc) {
    static const char* locationNames[] = {
        "NOT_ACCURATE",
        "NEAR_ROOM_232",
        "NEAR_ROOM_234",
        "BETWEEN_ROOMS_234_236",
        "ROOM_236",
        "ROOM_231",
        "BETWEEN_ROOMS_231_236",
        "NEAR_BATHROOM",
        "NEAR_KITCHEN",
        "KITCHEN",
        "MAIN_ENTRANCE",
        "NEAR_ROOM_230",
        "LOBBY",
        "ROOM_201",
        "PRINTER",
        "MAIN_EXIT",
        "BALCONY_ENTRANCE",
        "OFFICES_HALL"
    };

    if (loc < 0 || loc >= (sizeof(locationNames) / sizeof(locationNames[0]))) {
        return "UNKNOWN_LOCATION";
    }
    return locationNames[loc];
}


// ====================== State Conversion ======================

const char* systemStateToString(int state) {
    switch (state) {
        case STATIC_RSSI:             return "STATIC_RSSI";
        case STATIC_RSSI_TOF:         return "STATIC_RSSI_TOF";
        case STATIC_DYNAMIC_RSSI:     return "STATIC_DYNAMIC_RSSI";
        case STATIC_DYNAMIC_RSSI_TOF: return "STATIC_DYNAMIC_RSSI_TOF";
        case OFFLINE:                 return "OFFLINE";
        default:                      return "UNKNOWN_STATE";
    }
}

const char* modeToString(SystemMode mode) {
    switch (mode) {
        case MODE_TRAINING_ONLY:         return "Training";
        case MODE_PREDICTION_ONLY:       return "Prediction";
        case MODE_RSSI_MODEL_DIAGNOSTIC: return "Verify RSSI Anchors";
        case MODE_TOF_DIAGNOSTIC:        return "Verify TOF Responders";
        case MODE_FULL_SESSION:          return "Combined Prediction";
        default:                         return "Unknown Mode";
    }
}


// ====================== Utility Functions ======================

int applyEMA(int prevRSSI, int newRSSI) {
    if (prevRSSI == RSSI_DEFAULT_VALUE) return newRSSI;
    return (int)(ALPHA * prevRSSI + (1.0f - ALPHA) * newRSSI);
}

void resetReuseFromSDForLabel() {
    for(int i = 0; i < NUMBER_OF_LABELS; i++) {
        reuseFromSD[Label(i)] = false;
    }
}

void setReuseFromSDForLabel(Label label) {
    reuseFromSD[label] = true;
}

bool getShouldReuseForLabel(Label label) {
    return reuseFromSD[label];
}