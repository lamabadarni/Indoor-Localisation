//
// Created by halam on 21/05/2025.
//

#ifndef UNTITLED3_SCAN_DATA_MANAGER_H
#define UNTITLED3_SCAN_DATA_MANAGER_H


#include <Arduino.h>
#include <vector>

// Bring in SystemState (if you need it here)
#include "utilities.h"

// Captures your scan parameters for meta.txt
struct ScanConfig {
    int                 layoutVersion;
    std::vector<String> anchorMacs;
    int                 expectedSegments;
    int                 scansPerSegment;
    int                 featureCountRSSI;
    int                 featureCountTOF;
};

// Defined in main.ino
extern ScanConfig currentConfig;

// Read/write the meta.txt bookkeeping
bool loadMeta(const char* metaPath, ScanConfig& cfg);
bool metaMatches(const ScanConfig& stored, const ScanConfig& current);
void saveMeta(const char* metaPath, const ScanConfig& cfg, uint32_t lastScanUnix);

// Check that scans.csv has the full expected number of rows
bool isCSVComplete(const char* csvPath, const ScanConfig& cfg);
bool pruneIncompleteSegments(const char* csvPath, const ScanConfig& cfg, std::vector<int>& doneLocs);
bool shouldReuseScans(const char* metaPath, const char* csvPath,const ScanConfig& cfg);
bool verifyCSVFormat(const char* csvPath, const ScanConfig& cfg);
#endif //UNTITLED3_SCAN_DATA_MANAGER_H
