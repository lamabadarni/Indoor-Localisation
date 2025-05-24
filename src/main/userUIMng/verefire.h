/**
 * @file verifier.h
 * @brief Diagnostic scanning utilities for evaluating RSSI and TOF coverage across labels.
 *
 * This module provides tools to:
 * - Scan anchor visibility and signal strength (RSSI)
 * - Scan TOF responder reachability
 * - Prompt user for feedback and optionally abort flow
 */

#ifndef VERIFIER_H
#define VERIFIER_H

#include "utillities.h"

// ====================== RSSI Verification ======================

/**
 * @brief Perform RSSI scan for a given label.
 * @param label Target label.
 * @return Result struct with per-anchor RSSI data.
 */
RSSICoverageResult scanRSSICoverage(Label label);

/**
 * @brief Perform interactive diagnostic loop for verifying RSSI anchor coverage.
 * @return true if the user completed all labels successfully; false if user aborted early.
 */
bool verifyRSSIScanCoverage();


// ====================== TOF Verification ======================

/**
 * @brief Perform TOF scan for a given label.
 * @param label Target label.
 * @return Result struct with per-responder distance (in cm).
 */
TOFCoverageResult scanTOFCoverage(Label label);

/**
 * @brief Perform interactive diagnostic loop for verifying TOF responder coverage.
 * @return true if the user completed all labels successfully; false if user aborted early.
 */
bool verifyTOFScanCoverage();

#endif // VERIFIER_H
