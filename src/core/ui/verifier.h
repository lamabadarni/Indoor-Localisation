
#ifndef VEREFIER_H
#define VEREFIER_H

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/ui/logger.h"
#include "core/ui/userUI.h"

typedef struct {
    int    seen    = 0;
    double average = 0;
} Coverage;

// ====================== RSSI Verification ======================

bool interactiveScanCoverage();

/**
 * @brief Perform RSSI scan for a given label.
 * @return Result struct with per-anchor RSSI data.
 */
Coverage scanRSSICoverage();

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
Coverage scanTOFCoverage();

/**
 * @brief Verifies TOF responder visibility with user feedback.
 * @return true if user approves coverage at all tested labels.
 */
bool verifyTOFScanCoverage();

#endif // VEREFIER_H
