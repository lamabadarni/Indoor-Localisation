#ifndef SD_CARD_BACKUP_H
#define SD_CARD_BACKUP_H

#include <Arduino.h>
#include <vector>
#include "utilities.h"
#include "scan_data_manager.h"




/// @brief Initialize the SD card with retry logic.
/// @param csPin Chip-Select pin number for the SD card.
/// @return true if initialization was successful.
bool initSDCard(int csPin);

/// @brief Attempt a single SD card initialization.
/// @param csPin Chip-Select pin number.


/// @brief Load RSSI and/or TOF datasets from SD based on currentSystemState.
/// @return true if all required files loaded without errors.
bool loadLocationDataset();



/// @brief Update all relevant CSVs by filtering then reload data.
/// @return true if update and reload succeeded.
bool updateCSV();

/// @brief Append a single RSSI scan entry to the RSSI CSV.
/// @param row RSSIData struct containing the new scan.
/// @return true if write was successful.
bool saveRSSIScan(const RSSIData &row);

/// @brief Append a single TOF scan entry to the TOF CSV.
/// @param row TOFData struct containing the new scan.
/// @return true if write was successful.
bool saveTOFScan(const TOFData &row);



#endif // SD_CARD_BACKUP_H