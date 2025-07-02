#ifndef DYNAMIC_RSSI_SCANNER_H
#define DYNAMIC_RSSI_SCANNER_H

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "esp_wifi.h"

/**
 * @brief Performs a single RSSI scan with EMA smoothing.
 * 
 * Scans visible APs, matches known anchors, and computes EMA values.
 * Fills `accumulatedRSSIs` with smoothed values.
 * 
 * @return RSSIData containing current label and all anchor RSSIs.
 */
std::pair<DynamicRSSIData , DynamicMacData> createSingleDynamicRSSIScan();

/**
 * @brief Collects a batch of RSSI scans and saves them to internal storage.
 * 
 * Each scan is EMA-smoothed, stored, and logged.
 * Used for both training data collection and real-time prediction input.
 */
void performDynamicRSSIScan();

#endif // RSSI_SCANNER_H
