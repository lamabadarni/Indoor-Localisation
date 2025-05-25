/**

@file sdCardIO.cpp
@brief Handles SD card initialization, CSV loading/saving, and dataset filtering for RSSI and TOF data.
@author Hala Mahajna
*/

#include "sdCardIO.h"
#include "utils/platform.h"
#include "utils/logger.h"

static String CurrCsvPathToOpen = "";
static String metaPathToOpen = "";
static String accuracyPathToOpen = "";
static String tmp = ".tmp";

// Forward declarations
static bool loadFileToDataset(const String& path, bool isTofFile);
static bool verifyCSVFormat(const String& header, bool isTofFile);
static void splitByComma(String data, char comma);
static void fromCSVRssiToVector(String line);
static void fromCSVTofToVector(String line);
static bool deleteInvalidLocations(const String& filePath);

static std::vector<String> splitedString;

bool initSDCard() {
    if (!SD.begin(csPin)) {
        LOG_ERROR("SD", "SD init failed. Prompting user...");
        return promptUserSDCardInitializationApprove();
    }
    LOG_INFO("SD", "SD card initialized.");
    return true;
}

bool loadLocationDataset() {
    rssiDataSet.clear();
    tofDataSet.clear();
    bool ok = true;
    auto state = currentSystemState;

    // Load RSSI if required
    if (!loadFileToDataset(getRSSIFilePath(), false)) {
        ok = false;
    }

    // Load TOF if required
    if (state == SystemState::STATIC_RSSI_TOF || state == SystemState::STATIC_DYNAMIC_RSSI_TOF) {
        if (!loadFileToDataset(getTOFFilePath(), true)) {
            ok = false;
        }
    }

    if (!ok) LOG_ERROR("SD", "Error loading one or more dataset files.");
    return ok;
}

bool updateCSV() {
    bool ok = true;
    auto state = currentSystemState;

    if (state == SystemState::STATIC_RSSI_TOF || state == SystemState::STATIC_DYNAMIC_RSSI_TOF) {
        ok &= deleteInvalidLocations(getRSSIFilePath());
        ok &= deleteInvalidLocations(getTOFFilePath());
    } else if (state == SystemState::STATIC_RSSI || state == SystemState::STATIC_DYNAMIC_RSSI) {
        ok &= deleteInvalidLocations(getRSSIFilePath());
    }

    if (ok) return loadLocationDataset();
    
    LOG_ERROR("SD", "updateCSV failed.");
    return false;
}

bool deleteInvalidLocations(const String& filePath) {
    if (!SD.exists(filePath)) {
        LOG_WARN("SD", "File not found for filtering: %s", filePath.c_str());
        return false;
    }

    String tmpPath = filePath + tmp;
    File inputFile = SD.open(filePath, FILE_READ);
    File outputFile = SD.open(tmpPath, FILE_WRITE);
    if (!inputFile || !outputFile) {
        LOG_ERROR("SD", "Error opening files for filtering.");
        if (inputFile) inputFile.close();
        if (outputFile) outputFile.close();
        return false;
    }

    String header = inputFile.readStringUntil('\n');
    outputFile.println(header);

    while (inputFile.available()) {
        String line = inputFile.readStringUntil('\n');
        line.trim();
        if (line.length() < 3) continue;

        int lastComma = line.lastIndexOf(',');
        Label loc = (Label)line.substring(lastComma + 1).trim().toInt();
        if (validLocationsMap[loc]) {
            outputFile.println(line);
        }
    }

    inputFile.close();
    outputFile.close();

    SD.remove(filePath);
    SD.rename(tmpPath, filePath);
    LOG_INFO("SD", "Filtered file: %s", filePath.c_str());
    return true;
}

static bool loadFileToDataset(const String& path, bool isTofFile) {
    File f = SD.open(path, FILE_READ);
    if (!f) {
        LOG_ERROR("SD", "Failed to open for reading: %s", path.c_str());
        return false;
    }

    String header = f.readStringUntil('\n');
    header.trim();

    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() < 3) continue;
        if (isTofFile) {
            fromCSVTofToVector(line);
        } else {
            fromCSVRssiToVector(line);
        }
    }

    f.close();
    LOG_INFO("SD", "Successfully loaded data from: %s", path.c_str());
    return true;
}

bool saveRSSIScan(const RSSIData &row) {
    File f = SD.open(getRSSIFilePath(), FILE_APPEND);
    if (!f) return false;

    if (f.size() == 0) {
        for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i) {
            f.print(String(i) + "_rssi,");
        }
        f.println("Location");
    }

    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
        f.print(String(row.RSSIs[i]));
        f.print(',');
    }
    f.println(String((int)row.label));

    f.close();
    return true;
}

bool saveTOFScan(const TOFData &row) {
    File f = SD.open(getTOFFilePath(), FILE_APPEND);
    if (!f) return false;

    if (f.size() == 0) {
        for (int j = 1; j <= NUMBER_OF_RESPONDERS; ++j) {
            f.print(String(j) + "_tof,");
        }
        f.println("Location");
    }

    for (int j = 0; j < NUMBER_OF_RESPONDERS; ++j) {
        f.print(String(row.TOFs[j]));
        f.print(',');
    }
    f.println(String((int)row.label));

    f.close();
    return true;
}

static void fromCSVRssiToVector(String line) {
    RSSIData row{};
    splitByComma(line, ',');
    if (splitedString.size() != NUMBER_OF_ANCHORS + 1) return;

    for (int i = 0; i < NUMBER_OF_ANCHORS; i++) {
        row.RSSIs[i] = splitedString[i].toInt();
    }
    row.label = splitedString[NUMBER_OF_ANCHORS].toInt();
    rssiDataSet.push_back(row);
}

static void fromCSVTofToVector(String line) {
    TOFData row{};
    splitByComma(line, ',');
    if (splitedString.size() != NUMBER_OF_RESPONDERS + 1) return;

    for (int i = 0; i < NUMBER_OF_RESPONDERS; i++) {
        row.TOFs[i] = splitedString[i].toDouble();
    }
    row.label = splitedString[NUMBER_OF_RESPONDERS].toInt();
    tofDataSet.push_back(row);
}

static void splitByComma(String data, char comma) {
    splitedString.clear();
    int sepIndex;
    while ((sepIndex = data.indexOf(comma)) != -1) {
        splitedString.push_back(data.substring(0, sepIndex));
        data = data.substring(sepIndex + 1);
    }
    splitedString.push_back(data);
}
