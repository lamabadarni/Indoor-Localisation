#ifndef SD_CARD_UTILS_H
#define SD_CARD_UTILS_H

#include <Arduino.h>
#include "utillities.h" // For Data struct, LOCATIONS, TOTAL_APS, anchorSSIDs, dataSet
#include <vector>
#include "scan_data_manager.h"  // for currentConfig



/** Initialize SD, then report if SCAN_RESULTS_FILE already has all locations */
bool initSD(int csPin = 5);

/** True if SCAN_RESULTS_FILE exists and contains ≥1 row for every location */
bool hasAllLocationRows();


// Loads only the rows matching loc into outRows
bool loadLocationDataset(const char* path,LOCATIONS loc, std::vector<ScanData>& outRows);

// Verifies that the first line of csvPath matches the header implied
// by currentSystemState and currentConfig.anchorMacs
bool verifyCSVFormat(const char* csvPath);


bool saveLocationDataset(const String &path, const ScanData &row);

#endif // SD_CARD_H
