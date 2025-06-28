#pragma once

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "core/ui/userUI.h"

bool initDataBackup();

/**
 * @brief buffers the RSSI scan data for later saving.
 * @param scanData The RSSI scan data to be buffered.
 */
void saveData(const RSSIData &scanData);

/**
 * @brief buffers the TOF scan data for later saving.
 * @param scanData The TOF scan data to be buffered.
 */
void saveData(const TOFData &scanData);

/**
 * @brief Loads the dataset according to the current system state into the global vectors.
 * @return true if the dataset was loaded successfully, false otherwise.
 */
bool loadDataset(void);

/**
 * @brief Creates the RSSI/TOF directory according to the current system state if it does not exist,
 *        or recreates empty directories if they already exist.
 * @return true if the storage was formatted successfully, false otherwise.
 */
bool formatStorage(void);

/**
 * @brief Deletes invalid data based on the provided validMap.
 * @param validMap A mapping of Label -> valid data (true if valid, false otherwise).
 * @return true if the data was cleaned successfully, false otherwise.
 */
bool filterNonValidData(const bool validMap[LABELS_COUNT]);

/**
 * @brief Resets the CSV files for RSSI and TOF data and prints the column headers.
 * @return true if the reset was successful, false otherwise.
 */
bool resetCSV(void);

void doneCollectingData();