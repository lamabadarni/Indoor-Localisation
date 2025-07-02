/**
 * @file userUI.cpp
 * @brief Handles all user prompts for system configuration, scanning, validation, prediction, and abort flow.
 *
 * Each function is responsible for a specific interactive prompt and updates global config or state accordingly.
 * 
 */

#include "userUI.h"
#include "userUISerial.h"
#include "userUIOled.h"
// =======================================================
// 🟦 SYSTEM SETUP PROMPTS
// =======================================================

void runUserSystemSetup() {
    if(systemUI == OLED) {
        runUserSystemSetupOLED();
    }
    if(systemUI == SERIAL) {
        runUserSystemSetupSerial();
    }
}

void promptUserShowDebugLogs() {
    if(systemUI == OLED) {
        promptUserShowDebugLogsOLED();
    }
    if(systemUI == SERIAL) {
        promptUserShowDebugLogsSerial();
    }
}

// =======================================================
// 🟩 LABEL SELECTION
// =======================================================

void promptUserLocationLabel() {
    if(systemUI == OLED) {
        promptUserLocationLabelOLED();
    }
    if(systemUI == SERIAL) {
        promptUserLocationLabelSerial();
    }
}

void promptLabelsValidToPredection() {
    if(systemUI == OLED) {
        promptLabelsValidToPredectionOLED();
    }
    if(systemUI == SERIAL) {
        promptLabelsValidToPredectionSerial();
    }
}

void promptUserProceedToNextLabel() {
     if(systemUI == OLED) {
         promptUserProceedToNextLabelOLED();
    }
    if(systemUI == SERIAL) {
         promptUserProceedToNextLabelSerial();
    }
}

// =======================================================
// 🟨 BACKUP & REUSE
// =======================================================

bool promptUserReuseDecision() {
    if(systemUI == OLED) 
        return promptUserReuseDecisionOLED();
return promptUserReuseDecisionSerial();
    
}
// =======================================================
// 🟧 COVERAGE DIAGNOSTICS
// =======================================================

char promptUserRunCoverageDiagnostic() {
    if(systemUI == OLED) 
        return promptUserRunCoverageDiagnosticOLED();
     return promptUserRunCoverageDiagnosticSerial();
}

bool promptUserAbortToImproveEnvironment() {
    if(systemUI == OLED) 
        return promptUserAbortToImproveEnvironmentOLED();
return promptUserAbortToImproveEnvironmentSerial();
    
}

// =======================================================
// 🟥 VALIDATION PHASE
// =======================================================

bool promptUserRescanAfterInvalidation() {
    if(systemUI == OLED) {
        return promptUserRescanAfterInvalidationOLED();
    }
     return promptUserRescanAfterInvalidationSerial();
    
}

// =======================================================
// 🟪 PREDICTION PHASE
// =======================================================

bool promptUserApprovePrediction() {
    if(systemUI == OLED) {
       return  promptUserApprovePredictionOLED();
    }
    return promptUserApprovePredictionSerial();
    
}

Label promptUserChooseBetweenPredictions(Label left, Label right) {
    if(systemUI == OLED) 
        return promptUserChooseBetweenPredictionsOLED(left,right);

    return promptUserChooseBetweenPredictionsSerial(left,right);
    
}

bool promptUserRetryPredictionl() {
    if(systemUI == OLED) {
        return promptUserRetryPredictionOLED();
    }
       return promptUserRetryPredictionSerial(); 
}

bool promptUserForClearingDataAfterManyPredectionFailure() {
    if(systemUI == OLED) {
        return promptUserForClearingDataAfterManyPredectionFailureOLED();
    }
    if(systemUI == SERIAL) {
       return promptUserForClearingDataAfterManyPredectionFailureSerial();
    }
    return false;
}

char readCharFromUser(){

    if(systemUI == OLED) {
      return readCharFromUserOLED();
    }
       return readCharFromUserSerial();
}

bool promptUserRunAnotherSession(){
    if(systemUI == OLED) {
      return promptUserRunAnotherSessionOLED();
    }
    if(systemUI == SERIAL) {
       return promptUserRunAnotherSessionSerial();
    }
return false;
}