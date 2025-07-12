#pragma once

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "core/ui/userUI.h"
#include "core/ui/userUIOled.h"

bool initDataBackup(bool initSD);
/**
 * @brief buffers the Static RSSI scan data for later saving.
 * @param scanData The Static RSSI scan data to be buffered.
 */
void saveData(const StaticRSSIData &scanData);

/**
 * @brief buffers the TOF scan data for later saving.
 * @param scanData The TOF scan data to be buffered.
 */
void saveData(const TOFData &scanData);

/**
 * @brief buffers the Dynamic RSSI scan data for later saving.
 * @param scanData The Dynamic RSSI scan data to be buffered.
 */
void saveData(const DynamicMacData& macData, const DynamicRSSIData& scanData);
/**
 * @brief delete buffered data based DeleteInvalidData
 */
void deleteInvalidData(void);

/**
 * @brief Loads the dataset according to the current system state into the global vectors.
 * @return true if the dataset was loaded successfully, false otherwise.
 */
bool loadDataset(void);

/**
 * 
 */
bool loadBatch(void);

/**
 * @brief Creates the RSSI/TOF directory according to the current system state if it does not exist,
 *        or recreates empty directories if they already exist.
 * @return true if the storage was formatted successfully, false otherwise.
 */
bool formatStorage(bool format);

/**
 * @brief Resets the CSV files for RSSI and TOF data and prints the column headers.
 * @return true if the reset was successful, false otherwise.
 */
bool createCSV(void);

/**
 * @brief Assumes there are n buffered samples in the dataset that need to be saved to the file.
 */
void doneCollectingData();
