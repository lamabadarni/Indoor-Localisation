// sd_card.cpp

#include "sdCardBackup.h"
#include "utillities.h"
#include <SD.h>
#include <SPI.h>
#include <Arduino.h>
// File: sdCardBackup.cpp
#include "sdCardBackup.h"
#include <SPI.h>

static string csvPathToOpen = "";

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

bool loadLocationDataset() {

     verifyCSVFormat() 
    //switch case according to system state
    switch(Enablements::currentSystemState) {

        case(SystemState::STATIC_RSSI) {
        File f = SD.open(csvPath, FILE_READ);
        f.readStringUntil('\n');
        while (f.available()) {
        String line = f.readStringUntil('\n');
        Data d = fromCSV(line,metaPath);
        rssiDataSet.push_back(d);
     }
            break;
     }
        case(SystemState::STATIC_RSSI_TOF) {
        File f = SD.open(csvPath, FILE_READ);
         File f = SD.open(csvPath, FILE_READ);
        f.readStringUntil('\n');
        while (f.available()) {
        String line = f.readStringUntil('\n');
        Data d = fromCSV(line,metaPath);
        dataSet.push_back(d);
    }
            break;;
        }

    } 
    // save to csvPathToOpensv 
    File f = SD.open(csvPath, FILE_READ);
    if (!f) {
      //add log file not found
        return false;
    }
  if(verifyCSVFormat() == false) {
        return false;
    }

    // skip header
    f.readStringUntil('\n');
    while (f.available()) {
        String line = f.readStringUntil('\n');
        Data d = fromCSV(line,metaPath);
        dataSet.push_back(d);
    }
    f.close();
    return true;
}



bool verifyCSVFormat() {

  //make read since the file alrtready opened
      File f = SD.open(csvPath, FILE_READ);
    if (!f) return false;

    // Read first line as header
    String hdr = f.readStringUntil('\n');
    f.close();
    hdr.trim();

    // Build expected header tokens
    std::vector<String> expected;
    bool includeTOF = (currentConfig.systemState == SystemState::STATIC_RSSI_TOF ||
                       currentConfig.systemState == SystemState::STATIC_DYNAMIC_RSSI_TOF);

    // RSSI columns: 1_rssi, 2_rssi, ...
    for (int i = 1; i <= currentConfig.RSSINum; ++i) {
        expected.push_back(String(i) + "_rssi");
        // TOF columns: 1_tof, 2_tof, ... if applicable
        if (includeTOF) {
            expected.push_back(String(i) + "_tof");
        }
    }

    // Always end with Location and Timestamp
    expected.push_back("Location");
    expected.push_back("Timestamp");

    // Split actual header into tokens
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

    // Compare sizes and each token
    if (actual.size() != expected.size()) return false;
    for (size_t i = 0; i < actual.size(); ++i) {
        if (actual[i] != expected[i]) return false;
    }
    return true;
}

// Writes one Data row to 'CsvFilePath'. If the file is empty, writes a numeric header once.
bool saveLocationDataset(const String &CsvFilePath, const Data &row, ScanConfig currentConfig) {
    File f = SD.open(CsvFilePath.c_str(), FILE_WRITE);
    if (!f) {
        return false;
    }

    // Append this scan's CSV data
    f.println(toCSV(row,currentConfig));
    f.close();
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
    static Data fromCSV(const String &line, const char* metaPath) {
        Data row{};
        int start = 0, comma;
        int rssiCount = getRSSINumFromMeta(metaPath);
        int tofCount = getTOFNumFromMeta(metaPath);
        for (int i = 0; i < rssiCount; ++i) {
            comma = line.indexOf(',', start);
            row.RSSIs[i] = line.substring(start, comma).toInt();
            start = comma + 1;
        }
        for (int i = 0; i < tofCount; ++i) {
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

