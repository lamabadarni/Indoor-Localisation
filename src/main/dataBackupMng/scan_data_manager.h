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
 * @brief Create a new meta file if it doesn't already exist.
 */
bool createMetaFile(const String &filename);



#endif //UNTITLED3_SCAN_DATA_MANAGER_H
