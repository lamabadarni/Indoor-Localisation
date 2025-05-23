//
// Created by halam on 21/05/2025.
//

#ifndef UNTITLED3_SCAN_DATA_MANAGER_H
#define UNTITLED3_SCAN_DATA_MANAGER_H

#include <Arduino.h>
#include <vector>
#include "utilities.h"  // for SystemState, csvFormatMap
#include "sdCardBackup.h"
// Metadata structure capturing your scan configuration
/**
 * @brief Load meta configuration from a file into a ScanConfig object.
 */
bool loadMeta(const char* metaPath, ScanConfig& cfg);

/**
 * @brief Compare two ScanConfig instances for equality.
 */
bool metaMatches(const ScanConfig& stored, const ScanConfig& current);

/**
 * @brief Save a ScanConfig object into the meta file.
 */
void saveMeta(const char* metaPath, const ScanConfig& cfg);

/**
 * @brief Check if the CSV has the complete number of expected scan rows.
 */
bool isCSVComplete(const char* csvPath, const ScanConfig& cfg);

/**
 * @brief Determine whether the stored scans and meta data can be reused.
 */
bool shouldReuseScans(const char* metaPath, const char* csvPath, const ScanConfig& cfg);

/**
 * @brief Create a new meta file if it doesn't already exist.
 */
bool createMetaFile(const String &filename, const ScanConfig& cfg);

/**
 * @brief Extract the RSSI feature count from a meta file.
 */
int getRSSINumFromMeta(const char* metaPath);

/**
 * @brief Extract the TOF feature count from a meta file.
 */
int getTOFNumFromMeta(const char* metaPath);


#endif //UNTITLED3_SCAN_DATA_MANAGER_H
