/**
 * @file tofScanner.h
 * @brief Time-of-Flight (ToF) scanning module using FTM session initiation.
 *
 * Handles distance measurement to known responders via Wi-Fi Fine Timing Measurement (FTM).
 * Supports batch ToF scans, event callback handling, and prediction input generation.
 * 
 * Used in both scanning and validation phases.
 * 
 */

#ifndef TOF_SCANNER_H
#define TOF_SCANNER_H

#include "core/utils/utilities.h"
#include "core/utils/platform.h"
#include "core/utils/logger.h"
#include "esp_wifi.h"

/**
 * @brief Performs a batch of ToF scans to all configured responders.
 * 
 * Handles FTM session setup, waits for completion, and logs results.
 * Populates `accumulatedTOFs` and stores `TOFData` in internal memory.
 */
void performTOFScan();

/**
 * @brief Executes a single ToF scan across all responders.
 * 
 * Registers an event handler and measures round-trip time to each MAC.
 * 
 * @return TOFData containing distance estimates for all responders.
 */
TOFData createSingleTOFScan();

#endif // TOF_SCANNER_H
