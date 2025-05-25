/**
 * @file userUI.h
 * @brief UI prompts for user-driven configuration of system modes, enablements, and scan validation flow.
 *
 * Declares the interface for all user-interactive prompts, including:
 * - Mode/state selection
 * - Location label selection
 * - Accuracy approval, scan coverage evaluation
 * - Preference between RSSI and TOF predictions
 * 
 * These functions support the interactive setup and validation process of the localization system.
 * 
 * @author Lama Badarni
 */


#ifndef USER_UI_H
#define USER_UI_H

#include <Core/utils/utilities.h>  

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

/**
 * @brief Ask user if the predicted label is correct, retry, or skip.
 * @return 0 = incorrect, 1 = correct, 9 = skip
 */
int promptValidationApprovalOrSkip();

/**
 * @brief Ask user which prediction they trust when RSSI and TOF disagree.
 * @return 1 = trust RSSI, 2 = trust TOF, 0 = fallback to RSSI.
 */
int promptUserPreferredPrediction();


#endif // USER_UI_H
