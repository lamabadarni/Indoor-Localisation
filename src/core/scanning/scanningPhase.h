/**
 * @file scanningPhase.h
 * @brief Orchestrates the full scanning phase across all labels.
 *
 * Handles label-wise scanning with validation, coverage diagnostics, and retry logic.
 * Dispatches to RSSI/ToF scan modules depending on system scanner mode.
 * Used in training, validation, and full sessions.
 * 
 */

#ifndef SCANNING_PHASE_H
#define SCANNING_PHASE_H

#include "core/ui/userUI.h"
#include "core/ui/userUIOled.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"

/**
 * @brief Executes a full scan loop across all labels.
 *
 * - Prompts for each label.
 * - Offers reuse/validate options.
 * - Collects new scans and validates them.
 * - Skips failed scans after retries.
 */
void runScanningPhase();

/**
 * @brief Performs a scan session at the current label.
 *
 * - Optionally runs coverage diagnostics.
 * - Collects measurements and validates.
 * - Retries until accepted or max retries reached.
 *
 * @return true if scan passed validation, false if skipped.
 */
bool startLabelScanningSession();

/**
 * @brief Collects data using the active system scanner mode.
 *
 * Routes to either RSSI, ToF, or hybrid scanning logic.
 */
void collectMeasurements();

/**
 * @brief Collects a single RSSI+ToF snapshot without validation.
 *
 * Useful for diagnostics or testing.
 */
void createSingleScan();

/**
 * @brief Triggers a rescan after validation failure.
 *
 * Logs reason and re-collects measurements.
 */
void rescan();

#endif // SCANNING_PHASE_H
