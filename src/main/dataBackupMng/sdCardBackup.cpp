// sd_card.cpp

#include "sdCardBackup.h"
#include "utillities.h"
#include <SD.h>
#include <SPI.h>
#include <Arduino.h>
// File: sdCardBackup.cpp
#include "sdCardBackup.h"
#include <SPI.h>

static String CurrCsvPathToOpen = "";
static String metaPathToOpen = "";
static String accuracyPathToOpen = "";

bool initSDCard(int csPin) {
  for(int i = 0; i < MAX_RETRIES; i++) {
    if(tryToInitSD()) {
      return true;
    }
  }
  Serial.println("SD initialization failed after multiple attempts.");
  return false;
}

bool tryToInitSD() {
    if (!SD.begin()) {
        Serial.println("SD initialization failed!");
        return promptUserSDCardInitializationApprove();
    }
    Serial.println("SD card initialized.");
    return true;
}

bool verifyCSVFormat() {
    File f = SD.open(CurrCsvPathToOpen, FILE_READ);
    if (!f) return false;

    // Read and trim the header line
    String hdr = f.readStringUntil('\n');
    f.close();
    hdr.trim();

    // Detect whether this file contains TOF columns
    bool includeTOF = (hdr.indexOf("_tof") != -1);

    // Build expected header tokens
    std::vector<String> expected;
    if (includeTOF) {
        for (int i = 1; i <= NUMBER_OF_RESPONDERS; ++i)
            expected.push_back(String(i) + "_tof");
    } else {
        for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i)
            expected.push_back(String(i) + "_rssi");
    }
    expected.push_back("Location");
    expected.push_back("Timestamp");

    // Split actual header
    std::vector<String> actual;
    int start = 0;
    while (true) {
        int comma = hdr.indexOf(',', start);
        if (comma < 0) {
            actual.push_back(hdr.substring(start));
            break;
        }
        actual.push_back(hdr.substring(start, comma));
        start = comma + 1;
    }

    // Compare
    if (actual.size() != expected.size()) return false;
    for (size_t i = 0; i < actual.size(); ++i) {
        if (actual[i] != expected[i]) return false;
    }
    return true;
}

//------------------------------------------------------------------------------
// Load the scan dataset from SD into memory, based on currentSystemState
//------------------------------------------------------------------------------
bool loadLocationDataset() {
    switch (Enablements::currentSystemState) {
        case SystemState::STATIC_RSSI: {
            CurrCsvPathToOpen = getRSSIFilePath();
            if (!verifyCSVFormat()) return false;
            break;
        }
        case SystemState::STATIC_RSSI_TOF: {
            // first load RSSI file
            CurrCsvPathToOpen = getRSSIFilePath();
            if (!verifyCSVFormat()) return false;
            // then load TOF file
            CurrCsvPathToOpen = getTOFFilePath();
            if (!verifyCSVFormat()) return false;
            break;
        }
        default:
            Serial.println("Unsupported system state in loadLocationDataset()");
            return false;
    }

    // Now open whichever file we last set in CurrCsvPathToOpen
    CurrCsvPathToOpen=getRSSIFilePath();
    File f = SD.open(CurrCsvPathToOpen, FILE_READ);
    if (!f) return false;
    f.readStringUntil('\n');  // skip header
    while (f.available()) {
        String line = f.readStringUntil('\n');
        RSSIData d = fromCSVRssi(line);
            rssiDataSet.push_back(d);
    }
        f.close();
        if (Enablements::currentSystemState == SystemState::STATIC_RSSI_TOF) {

    CurrCsvPathToOpen=getTOFFilePath();
    File f = SD.open(CurrCsvPathToOpen, FILE_READ);
    if (!f) return false;
    f.readStringUntil('\n');  // skip header
    while (f.available()) {
        String line = f.readStringUntil('\n');
        TOFData d = fromCSVTof(line);
            tofDataSet.push_back(d);
    }
        f.close();   
} 
    return true;
}

bool DeleteinvalidLoc() {
    // 1) Open existing file for reading
    if (!SD.exists(CurrCsvPathToOpen)) {
        Serial.println("CSV does not exist: " + CurrCsvPathToOpen);
        return false;
    }
    File f = SD.open(CurrCsvPathToOpen, FILE_READ);
    if (!f) return false;

    // 2) Read and store the header line
    String header = f.readStringUntil('\n');
    header.trim();
    std::vector<String> validLines;
    validLines.push_back(header);  // ← this ensures header survives the rewrite

    // 3) Filter data rows
    while (f.available()) {
        String line = f.readStringUntil('\n');
        if (line.length() < 5) continue;

        int lastComma       = line.lastIndexOf(',');
        int secondLastComma = line.lastIndexOf(',', lastComma - 1);
        LOCATIONS loc = (LOCATIONS)line.substring(secondLastComma + 1, lastComma).toInt();

        if (validLocationsMap[loc]) {
            validLines.push_back(line);
        }
    }
    f.close();

    // 4) Rewrite file from scratch, starting with header
    File out = SD.open(CurrCsvPathToOpen, FILE_WRITE);
    if (!out) return false;
    out.seek(0);
    out.truncate();                 // wipe everything
    for (auto &ln : validLines) {
        out.println(ln);            // rewrite header + data
    }
    out.close();

    // 5) Reload in-memory dataset from that same (now filtered) CSV
    //    loadLocationDataset() must respect CurrCsvPathToOpen
    if (!loadLocationDataset()) {
        Serial.println("Error reloading dataset after DeleteinvalidLoc()");
        return false;
    }

    Serial.println("Filtered CSV + reloaded: " + CurrCsvPathToOpen);
    return true;
}

//------------------------------------------------------------------------------
// Call DeleteinvalidLoc() on each relevant CSV based on system state
//------------------------------------------------------------------------------
bool updateCSV() {
    if (validLocationsMap.empty()) {
        Serial.println("No valid locations map available.");
        return false;
    }

    switch (Enablements::currentSystemState) {
        case SystemState::STATIC_RSSI: {
            CurrCsvPathToOpen = getRSSIFilePath();
            if (!DeleteinvalidLoc()) return false;
            break;
        }
        case SystemState::STATIC_RSSI_TOF:
        case SystemState::STATIC_DYNAMIC_RSSI:
        case SystemState::STATIC_DYNAMIC_RSSI_TOF: {
            // first RSSI
            CurrCsvPathToOpen = getRSSIFilePath();
            if (!DeleteinvalidLoc()) return false;
            // then TOF
            CurrCsvPathToOpen = getTOFFilePath();
            if (!DeleteinvalidLoc()) return false;
            break;
        }
        default:
            Serial.println("Unsupported system state in updateCSV()");
            return false;
    }

    return true;
}

bool createCSVFile(const String &filename, ScanConfig currentConfig) {
    if (SD.exists(filename)) {
        Serial.println("CSV file already exists.");
        return false;
    }

    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to create CSV file.");
        return false;
    }
        // RSSI columns: 1_rssi,2_rssi,...
        for (int i = 1; i <= currentConfig.RSSINum; ++i) {
            file.print(i);
            file.print("_rssi,");
        }
        // TOF columns: 1_tof,2_tof,... if configured
        for (int j = 1; j <= currentConfig.TOFNum; ++j) {
            file.print(j);
            file.print("_tof,");
        }
        // Finally Location and Timestamp
        file.println("Location,Timestamp");
    // Example CSV header
    file.close();
    return true;
}

    // parse one CSV line
static RSSIData fromCSVRssi(const String &line){
        RSSIData row{};
        int start = 0, comma;
        for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
            comma = line.indexOf(',', start);
            row.RSSIs[i] = line.substring(start, comma).toInt();
            start = comma + 1;
        }

        comma = line.indexOf(',', start);
        row.location = (LOCATIONS)line.substring(start, comma).toInt();
        start = comma + 1;
        row.timestamp = line.substring(start).toInt();
        return row;
    }

static TOFData fromCSVTof(const String &line) {
    TOFData row{};
    int start = 0, comma;
    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        comma = line.indexOf(',', start);
        row.TOFs[i] = line.substring(start, comma).toInt();
        start = comma + 1;
    }
    comma = line.indexOf(',', start);
    row.location = (LOCATIONS)line.substring(start, comma).toInt();
    start = comma + 1;
    row.timestamp = line.substring(start).toInt();
    return row;
}

    /*
    // serialize to CSV
String toCSV(const ScanData &row, const ScanConfig &currentConfig) {
    String s;

    for (int i = 0; i < currentConfig.RSSINum; ++i) {
        s += String(row.RSSIs[i]);
        s += ',';
    }

    for (int i = 0; i < currentConfig.TOFNum; ++i) {
        s += String(row.TOFs[i]);
        s += ',';
    }

    s += String((int)row.location);
    s += ',';
    s += String(row.timestamp);

    return s;
}
*/

void deleteOldScanFiles() {
    File root = SD.open("/");
    while (true) {
        File file = root.openNextFile();
        if (!file) break;
        
        String name = file.name();
        if (name.startsWith("/scans_v") && (name.endsWith(".csv") || name.endsWith(".csv.meta"))) {
            Serial.print(" Deleting old file: ");
            Serial.println(name);
            SD.remove(name);
        }
        file.close();
    }
}

bool csvContainsTOF() {
    if (!csvPathToOpen) return false;

    String header = csvPathToOpen.readStringUntil('\n');
    header.trim();

    // Look for any column ending with _tof
    int index = 0;
    while ((index = header.indexOf("_tof", index)) != -1) {
        return true;
    }
    return false;
}

bool saveRSSIScan(const RSSIData &row) {
    // Open (or create) in append mode
    File f = SD.open(getRSSIFilePath(), FILE_WRITE);
    if (!f) return false;

    // If file empty, write header first
    if (f.size() == 0) {
        for (int i = 1; i <= NUMBER_OF_ANCHORS; ++i) {
            f.print(String(i) + "_rssi,");
        }
        f.println("Location,Timestamp");
    }

    // Write the RSSI values
    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
        f.print(row.RSSIs[i]);
        f.print(',');
    }
    // Then location label and a timestamp
    f.print((int)row.label);
    f.print(',');
    f.println(millis() / 1000);

    f.close();
    return true;
}

bool saveTOFScan(const TOFData &row) {
    File f = SD.open(getTOFFilePath(), FILE_WRITE);
    if (!f) return false;

    // If file empty, write header first
    if (f.size() == 0) {
        for (int j = 1; j <= NUMBER_OF_RESPONDERS; ++j) {
            f.print(String(j) + "_tof,");
        }
        f.println("Location,Timestamp");
    }

    // Write the TOF values
    for (int j = 0; j < NUMBER_OF_RESPONDERS; ++j) {
        f.print(row.TOFs[j]);
        f.print(',');
    }
    // Then location label and a timestamp
    f.print((int)row.label);
    f.print(',');
    f.println(millis() / 1000);

    f.close();
    return true;
}