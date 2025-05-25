/**
 * @file utilities.cpp
 * @brief Implementation of global enablements, utility functions, and configuration mappings 
 *        used throughout the indoor localization system.
 */

#include <utilities.h>

// ====================== Enablements ======================

bool Enablements::enable_training_model_on_host_machine = false;
bool Enablements::enable_SD_Card_backup                 = false;
bool Enablements::run_validation_phase                  = false;
bool Enablements::verify_responder_mac_mapping          = false;
bool Enablements::verify_rssi_anchor_mapping            = false;

// ======================   Globals    ======================

const char* anchorSSIDs[NUMBER_OF_ANCHORS] = {
        "234/236",
        "236",
        "231/236",
        "231", 
        "kitchen",
        "entrance",
        "lobby",
        "balcony",
        "201",
        "offices"
};

ScanConfig currentConfig = {
    .systemState      = currentSystemState,
    .RSSINum          = NUMBER_OF_ANCHORS,
    .TOFNum           = NUMBER_OF_RESPONDERS
};

SystemState currentSystemState     = OFFLINE;
SystemMode  currentSystemMode      = MODE_FULL_SESSION;
bool reuseFromSD[NUMBER_OF_LABELS] = {false};
double accuracy                    = -1;
Label currentLabel                 = NOT_ACCURATE;      
std::vector<RSSIData> rssiDataSet;
int accumulatedRSSIs[NUMBER_OF_ANCHORS];

// ====================== Label Conversion ======================

const char* labelToString(int loc) {
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


// ====================== Conversion ======================

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
// ======================Static fun  ======================
static void deleteDirectory(const String &dirPath);
String metaPath = getSDBaseDir() + META_FILENAME;
String rssiPath = getSDBaseDir() + RSSI_FILENAME;
String tofPath = getSDBaseDir() + TOF_FILENAME;
String accPath = getSDBaseDir() + ACCURACY_FILENAME;

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


static void deleteDirectory(const String &dirPath) {
    if (!SD.exists(dirPath)) return;

    File dir = SD.open(dirPath);
    while (File entry = dir.openNextFile()) {
        String entryName = entry.name();
        entry.close();

        String fullPath = dirPath + "/" + entryName;
        if (SD.exists(fullPath) && SD.open(fullPath).isDirectory()) {
            // sub-directory
            deleteDirectory(fullPath);
            SD.rmdir(fullPath);
        } else {
            // file
            SD.remove(fullPath);
        }
    }
    dir.close();

    // finally remove the now-empty directory
    SD.rmdir(dirPath);
    Serial.println("Deleted directory: " + dirPath);
}

/**
 * @brief Wipe and re-create the state directory, then create only the correct files.
 * @return true on success, false on any SD error.
 */
bool resetStorage() {
    // Build the directory name, e.g. "/STATIC_RSSI_TOF"
    String baseDir = "/" + String(systemStateToString(currentSystemState)) + "/";

    // 1) Delete existing state folder (and contents)
    deleteDirectory(baseDir);

    // 2) Recreate the empty folder
    if (!SD.mkdir(baseDir)) {
        Serial.println("Failed to mkdir: " + baseDir);
        return false;
    }
    Serial.println("Created directory: " + baseDir);

    /*
    // 3) Meta file (always)
    if (!createMetaFile(metaPath)) {
        Serial.println("Failed to create meta file.");
        return false;
    }
        */
       
    // 4) RSSI CSV (always)
    {
        File f = SD.open(rssiPath, FILE_WRITE);
        if (!f) {
            Serial.println("Failed to create RSSI CSV.");
            return false;
        }
        for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i) {
            f.print(String(i) + "_rssi,");
        }
        f.println("Location");
        f.close();
    }

    // 5) Accuracy CSV (always)
    {
        File f = SD.open(accPath, FILE_WRITE);
        if (!f) {
            Serial.println("Failed to create accuracy CSV.");
            return false;
        }
        f.println("Location,Accuracy");
        f.close();
    }

    // 6) TOF CSV (only in TOF modes)
    if (currentConfig.systemState == STATIC_RSSI_TOF ||
        currentConfig.systemState == STATIC_DYNAMIC_RSSI_TOF) {
        File f = SD.open(tofPath, FILE_WRITE);
        if (!f) {
            Serial.println("Failed to create TOF CSV.");
            return false;
        }
        for (int j = 1; j <= NUMBER_OF_RESPONDERS; ++j) {
            f.print(String(j) + "_tof,");
        }
        f.println("Location");
        f.close();
    }

    Serial.println("Storage reset complete for state: " +
                   String(systemStateToString(currentConfig.systemState)));
    return true;
}