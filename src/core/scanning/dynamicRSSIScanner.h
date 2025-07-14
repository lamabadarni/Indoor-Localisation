#ifndef DYNAMIC_RSSI_SCANNER_H
#define DYNAMIC_RSSI_SCANNER_H

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "esp_wifi.h"


std::pair<DynamicRSSIData , DynamicMacData> createSingleDynamicRSSIScan();
void performDynamicRSSIScan();

#endif // RSSI_SCANNER_H
