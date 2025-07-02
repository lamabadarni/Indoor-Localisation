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

bool promptUserProceedToNextLabel() {
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

char promptUserReuseDecisionSerial() {
    LOG_INFO("BACKUP", "[USER] > Reuse saved scan?");
    LOG_INFO("UI", "  Y - Yes, reuse");
    LOG_INFO("UI", "  N - No, rescan");
    return readCharFromUser();
}

// =======================================================
// 🟧 COVERAGE DIAGNOSTICS
// =======================================================

char promptUserRunCoverageDiagnostic() {
    if(systemUI == OLED) {
        promptUserRunCoverageDiagnosticOLED();
    }
    if(systemUI == SERIAL) {
        promptUserRunCoverageDiagnosticSerial();
    }
}

bool promptUserAbortToImproveEnvironment() {
    if(systemUI == OLED) {
        promptUserAbortToImproveEnvironmentOLED();
    }
    if(systemUI == SERIAL) {
        promptUserAbortToImproveEnvironmentSerial();
    }
}

// =======================================================
// 🟥 VALIDATION PHASE
// =======================================================

bool promptUserRescanAfterInvalidation() {
    if(systemUI == OLED) {
        promptUserRescanAfterInvalidationOLED();
    }
    if(systemUI == SERIAL) {
        promptUserRescanAfterInvalidationSerial();
    }
}

// =======================================================
// 🟪 PREDICTION PHASE
// =======================================================

bool promptUserApprovePrediction() {
    if(systemUI == OLED) {
        promptUserApprovePredictionOLED();
    }
    if(systemUI == SERIAL) {
        promptUserApprovePredictionSerial();
    }
}

Label promptUserChooseBetweenPredictions(Label left, Label right) {
    if(systemUI == OLED) {
        promptUserChooseBetweenPredictionsOLED();
    }
    if(systemUI == SERIAL) {
        promptUserChooseBetweenPredictionsSerial();
    }
}

bool promptUserRetryPredictionl() {
    if(systemUI == OLED) {
        promptUserRetryPredictionlOLED();
    }
    if(systemUI == SERIAL) {
        promptUserRetryPredictionlSerial();
    }
}

bool promptUserForClearingDataAfterManyPredectionFailure(Label label) {
    if(systemUI == OLED) {
        promptUserForClearingDataAfterManyPredectionFailureOLED();
    }
    if(systemUI == SERIAL) {
       promptUserForClearingDataAfterManyPredectionFailureSerial();
    }
}