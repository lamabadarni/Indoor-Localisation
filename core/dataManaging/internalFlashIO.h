
#ifndef INTERNAL_FLASH_IO_H
#define INTERNAL_FLASH_IO_H

#include <../utils/platform.h>
#include <../utils/utilities.h>

// ====================== Initialization ======================

bool initInternalFlash();

// ====================== Dataset Management ======================

bool loadLocationDataset();
bool cleanupCSV(const bool validLocationsMap[NUMBER_OF_LABELS]);
bool resetCSV();

// ====================== Scan Row Writing ======================

bool saveRSSIScan(const RSSIData& row);
bool saveTOFScan(const TOFData& row);

#endif // INTERNAL_FLASH_IO_H
