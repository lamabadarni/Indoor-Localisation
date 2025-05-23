#ifndef SD_CARD_UTILS_H
#define SD_CARD_UTILS_H

#include <Arduino.h>
#include "utilities.h" // For Data struct, LOCATIONS, TOTAL_APS, anchorSSIDs, dataSet
#include <vector>
#include "scan_data_manager.h"  // for currentConfig

const string RSSI_CSV_FILE_NAME = systemStateToString + "/RSSI_scan_data_.csv";
const string TOF_CSV_FILE_NAME = systemStateToString + "/TOF_scan_data_.csv";
const string META_FILE_NAME = systemStateToString + "/meta_.csv";
const string LOCATION_ACCURACY_FILE_NAME = systemStateToString + "/location_accuracy_.csv";

/**
 * @brief Initialize SD card on the given CS pin.
 */
bool initSDCard(int csPin = 5);

/**
 * @brief Load scan data from CSV into the global dataset.
 */
bool loadLocationDataset();

/**
 * @brief Save a single scan row to the CSV file.
 */
bool saveLocationDataset();

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
static Data fromCSV();

/**
 * @brief Serialize a ScanData structure to a CSV-formatted string.
 */
String toCSV(const ScanData &row, ScanConfig currentConfig);

/**
 * @brief Delete old CSV and .meta scan files from the SD card.
 */
void deleteOldScanFiles();
#endif // SD_CARD_UTILS_H
