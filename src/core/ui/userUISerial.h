#ifndef USER_UI_SERIAL_H
#define USER_UI_SERIAL_H

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"

// ======================== SYSTEM SETUP ========================

/**
 * @brief Runs initial user configuration sequence:
 *        log level, system mode, scanner mode, and toggles.
 */
void runUserSystemSetupSerial();

/**
 * @brief Prompts the user to show debug logs during rescan attempts.
 */
void promptUserShowDebugLogsSerial();

/**
 * @brief Prompts the user to run another session.
 */
bool promptUserRunAnotherSessionSerial();

// ======================== LOCATION LABELING ========================

/**
 * @brief Prompts the user to choose a label from the label list.
 */
void promptUserLocationLabelForScanSerial();

/**
 * @brief Displays all available labels before prediction begins.
 */
void promptLabelsValidToPredectionSerial();

/**
 * @brief Prompts whether to continue scanning other labels.
 * @return true to continue, false to exit or abort.
 */
void promptUserProceedToNextLabelSerial();

// ======================== BACKUP / RESTORE ========================

/**
 * @brief Prompts reuse decision for existing scan data.
 * @return 'Y', 'V', or 'N' based on user's choice.
 */

bool promptUserReuseDecisionSerial();

// ======================== COVERAGE CHECK ========================

/**
 * @brief Asks the user whether to run coverage diagnostic scan.
 * @return 'Y', 'N', or 'D' (Don't ask again).
 */
char promptUserRunCoverageDiagnosticSerial();

/**
 * @brief Asks if user wants to abort to improve environment before scanning.
 * @return true if abort is selected.
 */
bool promptUserAbortToImproveEnvironmentSerial();

// ======================== VALIDATION FLOW ========================

/**
 * @brief Asks if the user wants to rescan after validation fails.
 * @return true if rescan is approved.
 */
bool promptUserRescanAfterInvalidationSerial();

bool promptUserRetryValidationSerial();


// ======================== PREDICTION FLOW ========================

/**
 * @brief Asks user to approve prediction result.
 * @return true if prediction is accepted.
 */
bool promptUserApprovePredictionSerial();

/**
 * @brief Asks user to choose between two prediction options (RSSI vs ToF).
 * @param rssi The RSSI-predicted label.
 * @param tof The ToF-predicted label.
 * @return Chosen label, or LABELS_COUNT if rejected.
 */
Label promptUserChooseBetweenPredictionsSerial(Label left, Label right);

Label promptUserChooseBetweenTriplePredictionsSerial(Label first, Label second, Label third);

bool promptUserRetryPredictionSerial();

bool promptUserForClearingDataAfterManyPredectionFailureSerial();


// ======================== UTILS ========================

char readCharFromUserSerial();

int readIntFromUserSerial();

#endif 