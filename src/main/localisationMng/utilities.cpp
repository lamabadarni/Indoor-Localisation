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

// ======================   Globals    ======================

SystemState currentSystemState                          = OFFLINE;
SystemMode  currentSystemMode                           = MODE_FULL_SESSION;
bool shouldReuseBackup = false;



ScanConfig currentConfig = {
    .systemState      = currentSystemState,
    .RoundTimestamp   = 0,
    .RSSINum          = NUMBER_OF_ANCHORS,
    .TOFNum           = NUMBER_OF_RESPONDERS
};

bool reuseFromSD[NUMBER_OF_LABELS]                      = {false};
int scanAccuracy                                        = -1

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

/////////SD FILES Functions 
String getSDBaseDir() {
    return String("/") + systemStateToString(currentSystemState) + "/";
}

String getMetaFilePath() {
    return getSDBaseDir() + "meta_.csv";
}

String getRSSIFilePath() {
    return getSDBaseDir() + "rssi_scan_data_.csv";
}

String getTOFFilePath() {
    return getSDBaseDir() + "tof_scan_data_.csv";
}

String getAccuracyFilePath() {
    return getSDBaseDir() + "location_accuracy_.csv";
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

// Helper: delete a directory and all files within it, then remove the empty directory
static void deleteDirectory(const String &dirPath) {
    if (!SD.exists(dirPath)) return;
    File dir = SD.open(dirPath);
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) break;
        String name = entry.name();
        SD.remove(name);
        entry.close();
    }
    dir.close();
    SD.rmdir(dirPath);
    Serial.println("Deleted directory: " + dirPath);
}

/**
 * @brief Wipe out the entire SD subdirectory for the current state, then
 *        re-create it and lay down fresh CSV files with only headers.
 * @return true on success, false on any SD error.
 */
bool resetStorage() {
    String baseDir = getSDBaseDir();

    // 1) delete old
    deleteDirectory(baseDir);

    // 2) recreate directory
    if (!SD.mkdir(baseDir)) {
        Serial.println("Failed to create directory: " + baseDir);
        return false;
    }
    Serial.println("Created directory: " + baseDir);

    // 3) create meta file
    if (!createMetaFile(getMetaFilePath(), currentConfig)) {
        Serial.println("Failed to create meta file.");
        return false;
    }

    // 4) create RSSI CSV with header
    {
        File f = SD.open(getRSSIFilePath(), FILE_WRITE);
        if (!f) {
            Serial.println("Failed to create RSSI CSV.");
            return false;
        }
        // RSSI columns
        for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i) {
            f.print(String(i) + "_rssi,");
        }
        f.println("Location,Timestamp");
        f.close();
    }

    // 5) create accuracy CSV with header
    {
        File f = SD.open(getAccuracyFilePath(), FILE_WRITE);
        if (!f) {
            Serial.println("Failed to create accuracy CSV.");
            return false;
        }
        f.println("Location,Accuracy");
        f.close();
    }

    // 6) if TOF mode, create TOF CSV as well
    if (Enablements::currentSystemState == SystemState::STATIC_RSSI_TOF ||
        Enablements::currentSystemState == SystemState::STATIC_DYNAMIC_RSSI_TOF) {
        File f = SD.open(getTOFFilePath(), FILE_WRITE);
        if (!f) {
            Serial.println("Failed to create TOF CSV.");
            return false;
        }
        for (int j = 1; j <= NUMBER_OF_RESPONDERS; ++j) {
            f.print(String(j) + "_tof,");
        }
        f.println("Location,Timestamp");
        f.close();
    }

    Serial.println("Storage reset complete for state: " +
                   String(systemStateToString(currentConfig.systemState)));
    return true;
}