/**
 * @file userUI.h
 * @brief UI prompts for user-driven configuration of system modes, enablements, and active scan targets.
 *
 * This module handles all serial-based user interaction for selecting:
 * - System mode (training, prediction, diagnostics, etc.)
 * - Runtime enablement flags
 * - Scan labels, approval prompts, and abort conditions
 */

#ifndef USER_UI_H
#define USER_UI_H

#include "utillities.h"

// ====================== System Setup ======================

/**
 * @brief Prompt user to select the overall system mode.
 * @return Selected SystemMode enum.
 */
SystemMode promptSystemMode();

/**
 * @brief Prompt user to configure all system enablement flags.
 */
void setupEnablementsFromUser();

/**
 * @brief Prompt user to select the underlying scan type/state.
 * @return Selected SystemState enum.
 */
SystemState promptSystemState();


// ====================== Location Selection ======================

/**
 * @brief Prompt user to select a single location label.
 * @return Chosen Label enum.
 */
Label promptLocationLabel();

/**
 * @brief Prompt user to select a location for diagnostic purposes (as string).
 * @return Pointer to the selected location name string.
 */
char* promptLocationSelection();


// ====================== User Decision Prompts ======================

/**
 * @brief Prompt user to approve scan accuracy after scanning a label.
 * @return true if approved, false otherwise.
 */
bool promptUserAccuracyApprove();

/**
 * @brief Prompt user whether to approve RSSI scan coverage.
 * @return true if good coverage, false if bad.
 */
bool promptRSSICoverageUserFeedback();

/**
 * @brief Ask user if another label should be tested after a scan.
 * @return true to continue scanning another label.
 */
bool promptVerifyScanCoverageAtAnotherLabel();

/**
 * @brief Ask user if they want to abort and reconfigure anchor/responder placement.
 * @return true if abort is requested.
 */
bool promptAbortForImprovement();


// ====================== Other Prompts ======================

/**
 * @brief Ask user whether to retry or skip SD card initialization.
 * @return false = retry, true = skip.
 */
bool promptUserSDCardInitializationApprove();

/**
 * @brief Verify mapping between MAC addresses and expected anchor/responder names.
 */
void verifySystemConfiguration();

/**
 * @brief Prompt user to retry validation using only RSSI or only TOF.
 * @return 0 = abort, 1 = retry with RSSI, 2 = retry with TOF.
 */
int promptRetryValidationWithSingleMethod();

#endif // USER_UI_H
