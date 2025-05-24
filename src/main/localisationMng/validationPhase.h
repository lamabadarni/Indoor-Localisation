#ifndef IOT_INDOOR_LOCALISATION_VALIDATION_PHASE_H
#define IOT_INDOOR_LOCALISATION_VALIDATION_PHASE_H

#include "utillities.h"
#include <vector>


/**
 * @brief Runs the interactive validation phase for all defined labels.
 *
 * Prompts the user to select each label, performs scanning and prediction,
 * validates the prediction accuracy through user feedback, and summarizes the results.
 * This function iterates over all location labels and is typically used in training or testing modes.
 */
void runValidationPhase();

/**
 * @brief Starts the full interactive validation session.
 */
void runValidationPhase();

/**
 * @brief Internal: print final summary with all validation statuses.
 */
void printFinalValidationSummary();

/**
 * @brief Validates the scan results by checking how many predictions match the label.
 *        Uses both RSSI and TOF modules if enabled. If failed, offers fallback.
 */
bool validateScanAccuracy()

/*
    * @brief Determines the relevance of the backup dataset.
    *        Evaluates the dataset based on prediction accuracy.
    *        only called at setup() assumes that dataset is already filled
    *        with the backup dataset.
    * @return: True if the backup dataset meets the relevance criteria, false otherwise.
*/
bool isBackupDataSetRelevant(void);

#endif // IOT_INDOOR_LOCALISATION_VALIDATION_PHASE_H