/**
 * @file validationPhase.h
 * @brief Interface for validating predicted location labels via user confirmation.
 *
 * This module verifies the accuracy of prediction mechanisms (RSSI/ToF)
 * by comparing predicted labels to user-selected ground truth.
 * 
 * Includes:
 * - Full validation loop (`runValidationPhase`)
 * - Single-label retry/rescan validation (`startLabelValidationSession`)
 */

#ifndef IOT_INDOOR_LOCALISATION_VALIDATION_PHASE_H
#define IOT_INDOOR_LOCALISATION_VALIDATION_PHASE_H 

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "core/ui/userUI.h" 

/**
 * @brief Validates all labels interactively, one by one.
 * 
 * Logs results and tracks validation status in `validForPredection[]`.
 */
void runValidationPhase();

/**
 * @brief Predicts current label and asks user to confirm or retry.
 * 
 * Updates `validForPredection[currentLabel]` and logs results.
 */
void startLabelValidationSession();

#endif // IOT_INDOOR_LOCALISATION_VALIDATION_PHASE_H
