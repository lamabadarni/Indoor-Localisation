// sd_card.cpp

#include "sdCardBackup.h"
#include "utillities.h"
#include <SD.h>
#include <SPI.h>
#include <Arduino.h>

bool initSD(int csPin) {
    if (!SD.begin(csPin)) {
        Serial.println(" SD init failed");
        return false;
    }
    Serial.println("SD initialized");

    if (hasAllLocationRows()) {
        Serial.printf(" %s contains all %d locations\n",
                      SCAN_RESULTS_FILE, NUMBER_OF_LOCATIONS);
    } else {
        Serial.printf(" %s missing some locations\n",
                      SCAN_RESULTS_FILE);
    }
    return true;
}

bool hasAllLocationRows() {
    if (!SD.exists(SCAN_RESULTS_FILE)) return false;
    File f = SD.open(SCAN_RESULTS_FILE, FILE_READ);
    if (!f) return false;

    // skip header
    f.readStringUntil('\n');

    bool seen[NUMBER_OF_LOCATIONS+1] = { false };
    while (f.available()) {
        String line = f.readStringUntil('\n');
        if (line.length() < 5) continue;
        // skip RSSI columns
        int idx = 0, start = 0;
        for (; idx < TOTAL_APS; ++idx) {
            int c = line.indexOf(',', start);
            if (c < 0) break;
            start = c + 1;
        }
        // parse location field
        int c2 = line.indexOf(',', start);
        String tok = (c2<0 ? line.substring(start) : line.substring(start, c2));
        int loc = tok.toInt();
        if (loc >= 1 && loc <= NUMBER_OF_LOCATIONS) {
            seen[loc] = true;
        }
    }
    f.close();

    for (int i = 1; i <= NUMBER_OF_LOCATIONS; ++i) {
        if (!seen[i]) return false;
    }
    return true;
}

bool loadLocationDataset(LOCATIONS loc) {
    if (!SD.exists(SCAN_RESULTS_FILE)) {
        dataSet.clear();
        return false;
    }
    File f = SD.open(SCAN_RESULTS_FILE, FILE_READ);
    if (!f) return false;

    dataSet.clear();
    f.readStringUntil('\n');  // skip header
    while (f.available()) {
        String line = f.readStringUntil('\n');
        if (line.length() < 5) continue;
        Data d;
        int start = 0;
        for (int fld = 0; fld < TOTAL_APS + 2; ++fld) {
            int comma = line.indexOf(',', start);
            String tok = (comma<0 ? line.substring(start) : line.substring(start, comma));
            int val = tok.toInt();
            if      (fld < TOTAL_APS)    d.RSSIs[fld]   = val;
            else if (fld == TOTAL_APS)    d.location     = (LOCATIONS)val;
            else                          d.timestamp    = val;
            if (comma < 0) break;
            start = comma + 1;
        }
        if (d.location == loc) {
            dataSet.push_back(d);
        }
    }
    f.close();
    return true;
}

bool saveLocationDataset(LOCATIONS loc) {
    bool exists = SD.exists(SCAN_RESULTS_FILE);
    File f = exists
             ? SD.open(SCAN_RESULTS_FILE, FILE_APPEND)
             : SD.open(SCAN_RESULTS_FILE, FILE_WRITE);
    if (!f) return false;

    // write header if new file
    if (!exists) {
        for (int i = 0; i < TOTAL_APS; ++i) {
            f.printf("AnchorNum%d,", i);
        }
        f.println("Location,Timestamp");
    }

    // append only this location’s entries
    for (auto &d : dataSet) {
        if (d.location != loc) continue;
        for (int i = 0; i < TOTAL_APS; ++i) {
            f.printf("%d,", d.RSSIs[i]);
        }
        f.printf("%d,%d\n", int(d.location), d.timestamp);
    }
    f.close();
    return true;
}
bool verifyCSVFormat() {
    // If file doesn't exist yet, we consider format “valid” (it will be created correctly later)
    if (!SD.exists(SCAN_RESULTS_FILE)) return true;

    File f = SD.open(SCAN_RESULTS_FILE, FILE_READ);
    if (!f) return false;

    String header = f.readStringUntil('\n');
    f.close();

    // Build the expected header dynamically
    String expected;
    for (int i = 0; i < TOTAL_APS; ++i) {
        expected += "anchor" + String(i + 1) + ",";
    }
    expected += "Location,Timestamp";

    // Trim any trailing \r or whitespace
    header.trim();

    return (header == expected);
}