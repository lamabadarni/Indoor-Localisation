#ifndef SD_CARD_UTILS_H
#define SD_CARD_UTILS_H

#include <Arduino.h>
#include "utillities.h" // For Data struct, LOCATIONS, TOTAL_APS, anchorSSIDs, dataSet
#include <vector>




/** Initialize SD, then report if SCAN_RESULTS_FILE already has all locations */
bool initSD(int csPin = 5);

/** True if SCAN_RESULTS_FILE exists and contains ≥1 row for every location */
bool hasAllLocationRows();

bool loadLocationDataset(LOCATIONS loc);

bool saveLocationDataset(LOCATIONS loc);

bool verifyCSVFormat();


#endif // SD_CARD_H