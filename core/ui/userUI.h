#ifndef USER_UI_H
#define USER_UI_H

#include "../utils/utilities.h"

// ========== System Setup ==========
void runUserSystemSetup(); // <- wrapper for logger + mode + enablements

// ========== Location Selection ==========
void promptUserLocationLabel();

// ========== Scan Feedback ==========
bool promptUserApproveScanAccuracy();
bool promptUserRescanAfterInvalidation();
bool promptUserRetryValidation();
bool promptUserCoverageSufficient();
bool promptUserProceedToNextLabel();
bool promptUserAbortToImproveEnvironment();
void promptUserShowDebugLogs();

// ========== Prediction Feedback ==========
bool promptUserApprovePrediction();
Label promptUserChooseBetweenPredictions(Label rssi, Label tof);

// ========== Reuse or Abort Prompts ==========
char promptUserReuseDecision();
char promptUserRunCoverageDiagnostic();
void promptUserAbortOrContinue(bool allowAbort);

#endif // USER_UI_H
