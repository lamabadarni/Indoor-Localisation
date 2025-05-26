/**
 * @file verefier.h
 * @brief Diagnostic scanning utilities for evaluating RSSI coverage across labels.
 *
 * Provides functions to:
 * - Scan anchor visibility and RSSI levels
 * - Scan responder distance and visibility
 * - Run interactive feedback loops to guide placement tuning
 * 
 * Used for deployment diagnostics and anchor/responder optimization.
 * 
 * @author Lama Badarni
 */


#ifndef VEREFIER_H
#define VEREFIER_H

#include <utilities.h>

// ====================== RSSI Verification ======================

/**
 * @brief Perform RSSI scan for a given label.
 * @return Result struct with per-anchor RSSI data.
 */
double scanRSSICoverage();

/**
 * @brief Verifies anchor coverage via user feedback loop for RSSI.
 * @return true if user completed all checks, false if aborted.
 */
bool verifyRSSIScanCoverage();

#endif // VEREFIER_H
