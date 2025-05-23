// sd_card.cpp

#include "sdCardBackup.h"
#include "utillities.h"
#include <SD.h>
#include <SPI.h>
#include <Arduino.h>
// File: sdCardBackup.cpp
#include "sdCardBackup.h"
#include <SPI.h>


bool initSD(int csPin) {
    if (!SD.begin(csPin)) {
        Serial.println("⚠ SD init failed");
        return false;
    }
    Serial.println("✅ SD initialized");
    return true;
}

bool loadLocationDataset(const char* path,LOCATIONS loc, std::vector<ScanData>& outRows) {
    outRows.clear();
    if (!SD.exists(path)) return false;
    File f = SD.open(path, FILE_READ);
    if (!f) return false;

    // skip header
    f.readStringUntil('\n');
    while (f.available()) {
        String line = f.readStringUntil('\n');
        if (line.length() < 3) continue;
        ScanData d = ScanData::fromCSV(line);
        if (d.location == loc)
            outRows.push_back(d);
    }
    f.close();
    return true;
}

bool verifyCSVFormat(const char* csvPath) {
    if (!SD.exists(csvPath)) return true;

    File f = SD.open(csvPath, FILE_READ);
    if (!f) return false;
    String hdr = f.readStringUntil('\n');
    f.close();
    hdr.trim();

    // build expected header inline
    std::vector<String> expected;
    for (auto &mac : currentConfig.anchorMacs) {
        switch (currentSystemState) {
          case SystemState::STATIC_RSSI:
            expected.push_back(mac + "_rssi");       break;
          case SystemState::STATIC_RSSI_TOF:
            expected.push_back(mac + "_rssi");
            expected.push_back(mac + "_tof");        break;
          case SystemState::STATIC_DYNAMIC_RSSI:
            expected.push_back(mac + "_rssi");       break;
          case SystemState::STATIC_DYNAMIC_RSSI_TOF:
            expected.push_back(mac + "_rssi");
            expected.push_back(mac + "_tof");        break;
          case SystemState::OFFLINE:
            break;
        }
    }
    expected.push_back("Location");
    expected.push_back("Timestamp");

    // split actual header
    std::vector<String> actual;
    int start = 0;
    while (true) {
        int c = hdr.indexOf(',', start);
        if (c < 0) { actual.push_back(hdr.substring(start)); break; }
        actual.push_back(hdr.substring(start, c));
        start = c + 1;
    }

    if (actual.size() != expected.size()) return false;
    for (size_t i = 0; i < actual.size(); ++i) {
        if (actual[i] != expected[i]) return false;
    }
    return true;
}

bool saveLocationDataset(const String &path, const ScanData &row) {
    bool exist = SD.exists(path);
    File f = exist
             ? SD.open(path, FILE_APPEND)
             : SD.open(path, FILE_WRITE);
    if (!f) return false;

    // If new file, write header
    if (!exist) {
        // write header inline
        for (auto &mac : currentConfig.anchorMacs) {
            switch (currentSystemState) {
              case SystemState::STATIC_RSSI:
                f.print(mac + "_rssi,");    
                   break;
              case SystemState::STATIC_RSSI_TOF:
              case SystemState::STATIC_DYNAMIC_RSSI_TOF:
                f.print(mac + "_rssi,");
                f.print(mac + "_tof,");    
                   break;
              case SystemState::STATIC_DYNAMIC_RSSI:
                f.print(mac + "_rssi,");  
                     break;
              case SystemState::OFFLINE:
                break;
            }
        }
        f.println("Location,Timestamp");
    }

    // Write the data row
    f.println(row.toCSV());
    f.close();
    return true;
}
