//
// Created by halam on 24/05/2025.
//

#ifndef UNTITLED3_ACCURACY_DATA_MANGER_H
#define UNTITLED3_ACCURACY_DATA_MANGER_H

#include <Arduino.h>
#include "utilities.h" // For Data struct, LOCATIONS, TOTAL_APS, anchorSSIDs, dataSet
#include <vector>
#include "scan_data_manager.h"  // for currentConfig
#include "sdCardBackup.h"



bool loadLocationAccuracy();


bool saveLocationAccuracy();


#endif //UNTITLED3_ACCURACY_DATA_MANGER_H
