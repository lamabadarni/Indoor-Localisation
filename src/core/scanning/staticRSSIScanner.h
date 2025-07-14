/**
 * @file staticRSSIScanner.h
 * @brief RSSI-based scanning module for training and prediction input generation.
 *
 * Handles single and batched RSSI scans using Exponential Moving Average (EMA).
 * Populates global RSSI buffers and stores data for model training or inference.
 * 
 * Used in both scanning and validation phases.
 * 
*/

#ifndef STATIC_RSSI_SCANNER_H
#define STATIC_RSSI_SCANNER_H

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
StaticRSSIData createSingleStaticRSSIScan();

/**
 * @brief Collects a batch of RSSI scans and saves them to internal storage.
 * 
 * Each scan is EMA-smoothed, stored, and logged.
 * Used for both training data collection and real-time prediction input.
 */
void performStaticRSSIScan();

#endif // RSSI_SCANNER_H
