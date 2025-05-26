#include "fileUtils.h"

static constexpr char META_FILENAME[]        = "meta_.csv";
static constexpr char RSSI_FILENAME[]        = "rssi_scan_data_.csv";
static constexpr char TOF_FILENAME[]         = "tof_scan_data_.csv";
static constexpr char ACCURACY_FILENAME[]    = "location_accuracy_.csv";
static constexpr char TMP_SUFFIX[]           = ".tmp";

static void deleteDirectory(const string &dirPath);
string metaPath = getSDBaseDir() + META_FILENAME;
string rssiPath = getSDBaseDir() + RSSI_FILENAME;
string tofPath = getSDBaseDir() + TOF_FILENAME;
string accPath = getSDBaseDir() + ACCURACY_FILENAME;

/////////SD FILES Functions 
string getSDBaseDir() {
    return String("/") + systemStateToString(currentSystemState) + "/";
}

string getMetaFilePath() {
    return getSDBaseDir() + "meta_.csv";
}

string getRSSIFilePath() {
    return getSDBaseDir() + "rssi_scan_data_.csv";
}

string getTOFFilePath() {
    return getSDBaseDir() + "tof_scan_data_.csv";
}

string getAccuracyFilePath() {
    return getSDBaseDir() + "location_accuracy_.csv";
}

void resetReuseFromSDForLabel() {
    for(int i = 0; i < LABELS_COUNT; i++) {
        reuseFromSD[Label(i)] = false;
    }
}

void setReuseFromSDForLabel(Label label) {
    reuseFromSD[label] = true;
}

bool getShouldReuseForLabel(Label label) {
    return reuseFromSD[label];
}

static void deleteDirectory(const string &dirPath) {
    if (!SD.exists(dirPath)) return;

    File dir = SD.open(dirPath);
    while (File entry = dir.openNextFile()) {
        string entryName = entry.name();
        entry.close();

        string fullPath = dirPath + "/" + entryName;
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
    string baseDir = "/" + String(systemStateToString(currentSystemState)) + "/";

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