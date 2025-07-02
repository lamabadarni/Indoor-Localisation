#ifndef USER_UI_H
#define USER_UI_H

#include "core/utils/utilities.h"

// ======================== SYSTEM SETUP ========================
void runUserSystemSetup();
void promptUserShowDebugLogs();

// ======================== LOCATION LABELING ========================
void promptUserLocationLabel();
void promptLabelsValidToPredection();
void promptUserProceedToNextLabel();

// ======================== BACKUP / RESTORE ========================
char promptUserReuseDecision();

// ======================== COVERAGE CHECK ========================
char promptUserRunCoverageDiagnostic();
bool promptUserAbortToImproveEnvironment();

// ======================== VALIDATION FLOW ========================
bool promptUserRescanAfterInvalidation();
bool promptUserRetryValidation();

// ======================== PREDICTION FLOW ========================
bool promptUserApprovePrediction();
Label promptUserChooseBetweenPredictions(Label left, Label right);
Label promptUserChooseBetweenTriplePredictions(Label first, Label second, Label third);
bool promptUserRetryPrediction();
bool promptUserForClearingDataAfterManyPredectionFailure();

char readCharFromUser();

bool promptUserRunAnotherSession();

#endif // USER_UI_H
