#ifndef IOT_INDOOR_LOCALISATION_VALIDATION_PHASE_H
#define IOT_INDOOR_LOCALISATION_VALIDATION_PHASE_H

#include "utillities.h"
#include <vector>

/**
 * @brief Starts the full interactive validation session.
 */
void runValidationPhase();

/**
 * @brief Internal: print final summary with all validation statuses.
 */
void printFinalValidationSummary(const bool validatedLabels[], const std::vector<ValidationFailure>& failures);

/**
 * @brief Validates the scan results by checking how many predictions match the label.
 *        Uses both RSSI and TOF modules if enabled. If failed, offers fallback.
 */
bool validateScanAccuracy()

#endif // IOT_INDOOR_LOCALISATION_VALIDATION_PHASE_H