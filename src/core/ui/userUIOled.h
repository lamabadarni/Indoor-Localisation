
#include "core/utils/utilities.h"

// ======================== SYSTEM SETUP ========================

/**
 * @brief Runs initial user configuration sequence:
 *        log level, system mode, scanner mode, and toggles.
 */
void runUserSystemSetupOLED();

/**
 * @brief Prompts the user to show debug logs during rescan attempts.
 */
void promptUserShowDebugLogsOLED();

// ======================== LOCATION LABELING ========================

/**
 * @brief Prompts the user to choose a label from the label list.
 */
void promptUserLocationLabelOLED();

/**
 * @brief Displays all available labels before prediction begins.
 */
void promptLabelsValidToPredectionOLED();

/**
 * @brief Prompts whether to continue scanning other labels.
 * @return true to continue, false to exit or abort.
 */
void promptUserProceedToNextLabelOLED();

// ======================== BACKUP / RESTORE ========================

/**
 * @brief Prompts reuse decision for existing scan data.
 * @return 'Y', 'V', or 'N' based on user's choice.
 */
bool promptUserReuseDecisionOLED();

// ======================== COVERAGE CHECK ========================

/**
 * @brief Asks the user whether to run coverage diagnostic scan.
 * @return 'Y', 'N', or 'D' (Don't ask again).
 */
char promptUserRunCoverageDiagnosticOLED();

/**
 * @brief Asks if user wants to abort to improve environment before scanning.
 * @return true if abort is selected.
 */
bool promptUserAbortToImproveEnvironmentOLED();

// ======================== VALIDATION FLOW ========================

/**
 * @brief Asks if the user wants to rescan after validation fails.
 * @return true if rescan is approved.
 */
bool promptUserRescanAfterInvalidationOLED();

/**
 * @brief Placeholder for prompt after multiple prediction failures.
 * @return true if user confirms clearing data.
 */
bool promptUserForClearingDataAfterManyPredectionFailureOLED(); // To be implemented

// ======================== PREDICTION FLOW ========================

/**
 * @brief Asks user to approve prediction result.
 * @return true if prediction is accepted.
 */
bool promptUserApprovePredictionOLED();

/**
 * @brief Asks user to choose between two prediction options (RSSI vs ToF).
 * @param rssi The RSSI-predicted label.
 * @param tof The ToF-predicted label.
 * @return Chosen label, or LABELS_COUNT if rejected.
 */
Label promptUserChooseBetweenPredictionsOLED(Label rssi, Label tof);

/**
 * @brief Asks user whether to retry prediction after failure.
 * @return true to retry; false otherwise.
 */
bool promptUserRetryPredictionOLED();

// ======================== SYSTEM ABORT FLOW ========================

/**
 * @brief Handles user-driven abort or continue decision.
 *        Waits for confirmation or timeout.
 */
void promptUserAbortOrContinueOLED();

char readCharFromUserOLED();

bool promptUserRunAnotherSessionOLED();