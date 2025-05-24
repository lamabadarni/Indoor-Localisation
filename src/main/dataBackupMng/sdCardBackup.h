#ifndef SD_CARD_BACKUP_H
#define SD_CARD_BACKUP_H

#include <Arduino.h>
#include <vector>
#include "utilities.h"
#include "scan_data_manager.h"

/**
 * @brief Initialize the SD card (retries internally).
 * @param csPin   SPI CS pin (default 5).
 * @return true if SD.begin() succeeded.
 */
bool initSDCard(int csPin = 5);

/**
 * @brief Load the scan CSV (or TOF+RSSI) into memory.
 */
bool loadLocationDataset();

/**
 * @brief Delete and filter in-place based on validLocationsMap.
 */
bool updateCSV();

/**
 * @brief Create a fresh CSV with the header for this ScanConfig.
 */
bool createCSVFile(const String &filename, ScanConfig currentConfig);

/**
 * @brief Helper to parse one CSV line (rssi+loc+ts).
 */
static RSSIData fromCSVRssi(const String &line);

/**
 * @brief Helper to parse one CSV line (rssi+loc+ts).
 */
static TOFData fromCSVTof(const String &line);


/**
 * @brief Serialize a ScanData to a CSV line.
 */
String toCSVRssi(const RSSIData &row);

/**
 * @brief Serialize a ScanData to a CSV line.
 */
String toCSVTof(const TOFData &row);

/**
 * @brief Remove old scan files and their .meta counterparts.
 */
void deleteOldScanFiles();

/**
 * @brief Append one RSSI row (with header if new).
 */
bool saveRSSIScan(const RSSIData &row);

/**
 * @brief Append one TOF row (with header if new).
 */
bool saveTOFScan(const TOFData &row);

#endif // SD_CARD_BACKUP_H