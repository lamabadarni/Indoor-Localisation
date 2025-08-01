/**
 * @file userUI.cpp
 * @brief Central dispatcher for user interaction prompts (serial or OLED).
 */

#include "userUI.h"
#include "userUISerial.h"
#include "userUIOled.h"

// ======================== SYSTEM SETUP ========================

void runUserSystemSetup() {
    // if(systemUI == OLED)   runUserSystemSetupOLED();
    if(systemUI == SERIAL) runUserSystemSetupSerial();
}

void promptUserShowDebugLogs() {
    // if(systemUI == OLED)   promptUserShowDebugLogsOLED();
    if(systemUI == SERIAL) promptUserShowDebugLogsSerial();
}

bool promptUserRunAnotherSession() {
    if(systemUI == SERIAL) return promptUserRunAnotherSessionSerial();
    return false;
}

// ======================== LOCATION LABELING ========================

void promptUserLocationLabelForScan() {
    // if(systemUI == OLED)   promptUserLocationLabelForScanOLED();
    if(systemUI == SERIAL) promptUserLocationLabelForScanSerial();
}

void promptLabelsValidToPredection() {
    // if(systemUI == OLED)   promptLabelsValidToPredectionOLED();
    if(systemUI == SERIAL) promptLabelsValidToPredectionSerial();
}

void promptUserProceedToNextLabel() {
    // if(systemUI == OLED)   promptUserProceedToNextLabelOLED();
    if(systemUI == SERIAL) promptUserProceedToNextLabelSerial();
}

// ======================== BACKUP / RESTORE ========================

char promptUserReuseDecision() {
    // if(systemUI == OLED)   return promptUserReuseDecisionOLED();
    if(systemUI == SERIAL) return promptUserReuseDecisionSerial();
    return 'N';
}

// ======================== COVERAGE CHECK ========================

char promptUserRunCoverageDiagnostic() {
    // if(systemUI == OLED)   return promptUserRunCoverageDiagnosticOLED();
    if(systemUI == SERIAL) return promptUserRunCoverageDiagnosticSerial();
    return 'N';
}

bool promptUserAbortToImproveEnvironment() {
    // if(systemUI == OLED)  return promptUserAbortToImproveEnvironmentOLED();
    if(systemUI == SERIAL) return promptUserAbortToImproveEnvironmentSerial();
    return false;
}

// ======================== VALIDATION FLOW ========================

bool promptUserRescanAfterInvalidation() {
    // if(systemUI == OLED)   return promptUserRescanAfterInvalidationOLED();
    if(systemUI == SERIAL) return promptUserRescanAfterInvalidationSerial();
    return false;
}

bool promptUserRetryValidation() {
    // if(systemUI == OLED)   return promptUserRetryValidationOLED(); ! HALA !
    if(systemUI == SERIAL) return promptUserRetryValidationSerial();
    return false;
}

// ======================== PREDICTION FLOW ========================

Label promptUserChooseBetweenPredictions(Label left, Label right) {
    // if(systemUI == OLED)  return promptUserChooseBetweenPredictionsOLED(left, right);
    if(systemUI == SERIAL) return promptUserChooseBetweenPredictionsSerial(left, right);
    return LABELS_COUNT;
}

Label promptUserChooseBetweenTriplePredictions(Label first, Label second, Label third) {
    // if(systemUI == OLED)  return promptUserChooseBetweenTriplePredictionsOLED(first, second, third); ! HALA !
    if(systemUI == SERIAL) return promptUserChooseBetweenTriplePredictionsSerial(first, second, third);
    return LABELS_COUNT;
}

bool promptUserRetryPrediction() {
    // if(systemUI == OLED)  return promptUserRetryPredictionOLED();
    if(systemUI == SERIAL) return promptUserRetryPredictionSerial();
    return false;
}

bool promptUserForClearingDataAfterManyPredectionFailure() {
    // if(systemUI == OLED)  return promptUserForClearingDataAfterManyPredectionFailureOLED();
    if(systemUI == SERIAL) return promptUserForClearingDataAfterManyPredectionFailureSerial();
    return false;
}

// ======================== UTILS ========================
char readCharFromUser() {
    // if(systemUI == OLED)   return readCharFromUserSerial();
    if(systemUI == SERIAL) return readCharFromUserSerial();
    return ' ';
}
