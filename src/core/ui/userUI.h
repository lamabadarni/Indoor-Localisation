/**
 * @file userUI.h
 * @brief User Interface prompts and system configuration input handlers.
 *
 * This module encapsulates all user-facing prompts used throughout
 * the system: system setup, label selection, scanning, prediction confirmation,
 * and retry/abort flows. All user interaction logic is centralized here.
 * 
 * Designed to support structured flow control during scanning and prediction sessions.
 * 
 */

#ifndef USER_UI_H
#define USER_UI_H

#include "core/utils/utilities.h"

// ======================== SYSTEM SETUP ========================

/**
 * @brief Runs initial user configuration sequence:
 *        log level, system mode, scanner mode, and toggles.
 */
void runUserSystemSetup();

/**
 * @brief Prompts the user to show debug logs during rescan attempts.
 */
void promptUserShowDebugLogs();

// ======================== LOCATION LABELING ========================

/**
 * @brief Prompts the user to choose a label from the label list.
 */
void promptUserLocationLabel();

/**
 * @brief Displays all available labels before prediction begins.
 */
void promptLabelsValidToPredection();

/**
 * @brief Prompts whether to continue scanning other labels.
 * @return true to continue, false to exit or abort.
 */
bool promptUserProceedToNextLabel();

// ======================== BACKUP / RESTORE ========================

/**
 * @brief Prompts reuse decision for existing scan data.
 * @return 'Y', 'V', or 'N' based on user's choice.
 */
char promptUserReuseDecision();

// ======================== COVERAGE CHECK ========================

/**
 * @brief Asks the user whether to run coverage diagnostic scan.
 * @return 'Y', 'N', or 'D' (Don't ask again).
 */
char promptUserRunCoverageDiagnostic();

/**
 * @brief Asks if user wants to abort to improve environment before scanning.
 * @return true if abort is selected.
 */
bool promptUserAbortToImproveEnvironment();

// ======================== VALIDATION FLOW ========================

/**
 * @brief Asks if the user wants to rescan after validation fails.
 * @return true if rescan is approved.
 */
bool promptUserRescanAfterInvalidation();

bool promptUserRetryValidation(); //HALA-IMPLEMENT

// ======================== PREDICTION FLOW ========================

/**
 * @brief Asks user to approve prediction result.
 * @return true if prediction is accepted.
 */
bool promptUserApprovePrediction();

/**
 * @brief Asks user to choose between two prediction options (RSSI vs ToF).
 * @param rssi The RSSI-predicted label.
 * @param tof The ToF-predicted label.
 * @return Chosen label, or LABELS_COUNT if rejected.
 */
Label promptUserChooseBetweenPredictions(Label rssi, Label tof);

bool promptUserRetryPrediction();

bool promptUserForClearingDataAfterManyPredectionFailure();

#endif // USER_UI_H
