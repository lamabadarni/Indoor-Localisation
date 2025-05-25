/**
 * @file verefier.h
 * @brief Diagnostic scanning utilities for evaluating RSSI and TOF coverage across labels.
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

#include <Core/utils/utilities.h>  

// ====================== RSSI Verification ======================

/**
 * @brief Perform RSSI scan for a given label.
 * @return Result struct with per-anchor RSSI data.
 */
RSSICoverageResult scanRSSICoverage();

/**
 * @brief Verifies anchor coverage via user feedback loop for RSSI.
 * @return true if user completed all checks, false if aborted.
 */
bool verifyRSSIScanCoverage();


// ====================== TOF Verification ======================

/**
 * @brief Measures TOF distance for each responder from the current label.
 * @return TOFCoverageResult with distances and visibility per responder.
 */
TOFCoverageResult scanTOFCoverage();

/**
 * @brief Verifies TOF responder visibility with user feedback.
 * @return true if user approves coverage at all tested labels.
 */
bool verifyTOFScanCoverage();

#endif // VEREFIER_H
