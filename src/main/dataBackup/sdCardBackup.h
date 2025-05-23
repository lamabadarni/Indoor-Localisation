#ifndef SD_CARD_UTILS_H
#define SD_CARD_UTILS_H

#include <Arduino.h>
#include "utilities.h" // For Data struct, LOCATIONS, TOTAL_APS, anchorSSIDs, dataSet
#include <vector>
#include "scan_data_manager.h"  // for currentConfig


/**
 * @brief Initialize SD card on the given CS pin.
 */
bool initSD(int csPin = 5);

/**
 * @brief Load scan data from CSV into the global dataset.
 */
bool loadLocationDataset(const char* csvPath, const char* metaPath, ScanConfig currentConfig);

/**
 * @brief Save a single scan row to the CSV file.
 */
bool saveLocationDataset(const String &path, const ScanData &row, ScanConfig currentConfig);

/**
 * @brief Create a new CSV file with the appropriate header based on ScanConfig.
 */
bool createCSVFile(const String &filename, ScanConfig currentConfig);

/**
 * @brief Validate that the CSV file header matches the expected structure.
 */
bool verifyCSVFormat(const char* csvPath, ScanConfig currentConfig);

/**
 * @brief Convert a CSV-formatted string into a Data structure.
 */
static Data fromCSV(const String &line, const char* metaPath);

/**
 * @brief Serialize a ScanData structure to a CSV-formatted string.
 */
String toCSV(const ScanData &row, ScanConfig currentConfig);

/**
 * @brief Delete old CSV and .meta scan files from the SD card.
 */
void deleteOldScanFiles();
#endif // SD_CARD_UTILS_H
