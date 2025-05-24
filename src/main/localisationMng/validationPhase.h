#ifndef IOT_INDOOR_LOCALISATION_VALIDATION_PHASE_H
#define IOT_INDOOR_LOCALISATION_VALIDATION_PHASE_H

#include "utillities.h"
#include <vector>

/**
 * @brief Struct for logging failed validations and reasons.
 */
struct ValidationFailure {
    Label label;
    const char* reason;
};

/**
 * @brief Starts the full interactive validation session.
 */
void runValidationPhase();

/**
 * @brief Checks whether a specific label has been validated.
 */
bool isLocationDataValid(Label location);

/**
 * @brief Internal: print validation warning with reason.
 */
void printValidationWarning(Label label, const char* reason);

/**
 * @brief Internal: print final summary with all validation statuses.
 */
void printFinalValidationSummary(const bool validatedLabels[], const std::vector<ValidationFailure>& failures);

/*
    * @brief Validates the location data.
    * @param location: The location data to be validated.
    * @return: True if the location data is valid, false otherwise.
    * @details This function checks if the location data is valid based on certain criteria.
    *          The criteria for validation can be defined here.
*/
bool isLocationDataValid(Label location) {}

#endif // IOT_INDOOR_LOCALISATION_VALIDATION_PHASE_H