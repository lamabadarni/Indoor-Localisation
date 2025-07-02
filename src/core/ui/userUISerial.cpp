/**
 * @file userUI.cpp
 * @brief Handles all user prompts for system configuration, scanning, validation, prediction, and abort flow.
 *
 * Each function is responsible for a specific interactive prompt and updates global config or state accordingly.
 * 
 */

#include "userUISerial.h"
#include "userUI.h"
// =======================================================
// 🟦 SYSTEM SETUP PROMPTS
// =======================================================

static void promptUserLoggerConfiguration() {
    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("SETUP", "[USER] > Select log level:");
    LOG_INFO("SETUP", "  I - ERROR and INFO logs");
    LOG_INFO("SETUP", "  D - ERROR, INFO, and DEBUG logs");

    char choosen = readCharFromUser();
    delay_ms(USER_PROMPTION_DELAY);
    if (choosen == 'i' || choosen == 'I') {
        SystemSetup::logLevel = LogLevel::LOG_LEVEL_INFO;
    } else if (choosen == 'd' || choosen == 'D') {
        SystemSetup::logLevel = LogLevel::LOG_LEVEL_DEBUG;
    }
}

static SystemMode promptUserSystemMode() {
    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("SETUP", "[USER] > Select System Mode:");
    for (int i = 0; i < MODES_NUM; ++i) {
        LOG_INFO("SETUP", "  %d - %s", i + 1, systemModes[i].c_str());
    }

    int sel = -1;
    while (sel < 1 || sel > MODES_NUM) sel = readIntFromUser();
    return static_cast<SystemMode>(sel - 1);
}

static SystemBootMode promptUserSystemBootMode() {
    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("SETUP", "[USER] > Select System Boot Mode:");
    for (int i = 0; i < SYSTEM_BOOT_MODES_NUM; ++i) {
        LOG_INFO("SETUP", "  %d - %s", i + 1, systemBootModes[i].c_str());
    }

    int sel = -1;
    while (sel < 1 || sel > SYSTEM_BOOT_MODES_NUM) sel = readIntFromUser();
    return static_cast<SystemBootMode>(sel - 1);
}

static SystemScannerMode promptUserScannerMode() {
    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("SETUP", "[USER] > Select Scanner Mode:");
    for (int i = 0; i < SYSTEM_SCANNER_MODES_NUM; ++i) {
        LOG_INFO("SETUP", "  %d - %s", i + 1, systemScannerModes[i].c_str());
    }

    int sel = -1;
    while (sel < 1 || sel > SYSTEM_SCANNER_MODES_NUM) sel = readIntFromUser();
    return static_cast<SystemScannerMode>(sel - 1);
}

static SystemPredictionMode promptUserPredictionMode() {
    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("SETUP", "[USER] > Select Prediction Mode:");
    for (int i = 0; i < SYSTEM_PREDICTION_NODES_NUM; ++i) {
        LOG_INFO("SETUP", "  %d - %s", i + 1, systemPredictionModes[i].c_str());
    }

    int sel = -1;
    while (sel < 1 || sel > SYSTEM_PREDICTION_NODES_NUM) sel = readIntFromUser();
    return static_cast<SystemPredictionMode>(sel - 1);
}

void runUserSystemSetupSerial() {
    promptUserLoggerConfiguration();
    SystemSetup::currentSystemMode = promptUserSystemMode();

    if (SystemSetup::currentSystemMode == MODE_SYSTEM_BOOT) {
        SystemSetup::currentSystemBootMode = promptUserSystemBootMode();
        return;
    }

    if( SystemSetup::currentSystemMode == MODE_SCANNING_SESSION || 
        SystemSetup::currentSystemMode == MODE_FULL_SESSION ) {
            SystemSetup::currentSystemScannerMode = promptUserScannerMode();
    }

    if (SystemSetup::currentSystemMode == MODE_PREDICTION_SESSION ||
        SystemSetup::currentSystemMode == MODE_FULL_SESSION) {
            SystemSetup::currentSystemPredictionMode = promptUserPredictionMode();
    }

    struct {
        const char* prompt;
        bool* flag;
    } toggles[] = {
        {"Enable validation phase? (y/n): ", &SystemSetup::enableValidationPhase},
    };

    for (auto& opt : toggles) {
        LOG_INFO("SETUP", "[USER] > %s", opt.prompt);
        *opt.flag = (readCharFromUser() == 'y' || readCharFromUser() == 'Y');
    }
}

void promptUserShowDebugLogsSerial() {
    LOG_INFO("FEEDBACK", "[USER] > Show debug logs during rescan? (y/n): ");
    char c = readCharFromUser();
    if (c == 'y' || c == 'Y') {
        SystemSetup::logLevel = LogLevel::LOG_LEVEL_DEBUG;
    }
}

bool promptUserRunAnotherSessionSerial() {
        LOG_INFO("MAIN", "Would you like to run another session?");
        LOG_INFO("MAIN", "(y - yes | n - no)");
        char again = readCharFromUser();
        if (again != 'y' && again != 'Y') {
            return true;
        }
return false;
}

// =======================================================
// 🟩 LABEL SELECTION
// =======================================================

void promptUserLocationLabelForScanSerial() {
    LOG_INFO("LABEL", "[USER] > Select label by index:");
    for (int i = 0; i < LABELS_COUNT; ++i) {
        LOG_INFO("LABEL", "  %d - %s", i + 1, labels[i].c_str());
    }

    int sel = -1;
    while (sel < 1 || sel > LABELS_COUNT) {
        sel = readIntFromUser();        
    }
    
    currentLabel = static_cast<Label>(sel - 1);
}

void promptLabelsValidPredectionSerial() {
    //LAMA TODO
    LOG_INFO("LABEL", "[USER] Labels valid for prediction:");
    for (int i = 0; i < LABELS_COUNT; ++i) {
        LOG_INFO("LABEL", "  %d - %s", i + 1, labels[i].c_str());
    }
}

void promptUserProceedToNextLabelSerial() {
    LOG_INFO("LABEL", "[USER] > Proceed to another label? (y/n): ");
    char c = readCharFromUser();
    shouldAbort = (c == 'N' || c == 'n'); //HALA : SEE CHANGE
}

// =======================================================
// 🟨 BACKUP & REUSE
// =======================================================

bool promptUserReuseDecisionSerial() {
    LOG_INFO("BACKUP", "[USER] > Reuse saved scan?");
    LOG_INFO("UI", "  Y - Yes, reuse");
    LOG_INFO("UI", "  N - No, rescan");
    char c = readCharFromUser();
    if (c == 'y' || c == 'Y') return true;
    return false;
}

// =======================================================
// 🟧 COVERAGE DIAGNOSTICS
// =======================================================

char promptUserRunCoverageDiagnosticSerial() {
    LOG_INFO("SCAN", "[USER] > Run scan coverage diagnostic before scanning?");
    LOG_INFO("UI", "  Y - Yes");
    LOG_INFO("UI", "  N - No");
    LOG_INFO("UI", "  D - Don't ask again");
    return readCharFromUser();
}

bool promptUserAbortToImproveEnvironmentSerial() {
    LOG_INFO("COVERAGE", "[USER] > Abort to improve environment? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

// =======================================================
// 🟥 VALIDATION PHASE
// =======================================================

bool promptUserRetryValidationSerial() {
    LOG_INFO("PREDICT", "[USER] > Retry validation? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y' ;
}

bool promptUserRescanAfterInvalidationSerial() {
    LOG_INFO("VALIDATE", "[USER] > Rescan after failed validation? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

// =======================================================
// 🟪 PREDICTION PHASE
// =======================================================

bool promptUserApprovePredictionSerial() {
    LOG_INFO("PREDICT", "[USER] > Approve predicted label? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

Label promptUserChooseBetweenPredictionsSerial(Label left, Label right) {
    LOG_INFO("PREDICT", "[USER] > Choose the correct prediction:");
    LOG_INFO("PREDICT", "  1 -  (%s)", labels[left].c_str());
    LOG_INFO("PREDICT", "  2 -  (%s)", labels[right].c_str());
    LOG_INFO("PREDICT", "  X - Both predictions are invalid");

    int sel = readIntFromUser();
    if (sel == 1) return left;
    if (sel == 2) return right;
    return LABELS_COUNT;
}

Label promptUserChooseBetweenTriplePredictionsSerial(Label first, Label second, Label third) {
    LOG_INFO("PREDICT", "[USER] > Choose the correct prediction:");
    LOG_INFO("PREDICT", "  1 - (%s)", labels[first].c_str());
    LOG_INFO("PREDICT", "  2 - (%s)", labels[second].c_str());
    LOG_INFO("PREDICT", "  3 - (%s)", labels[third].c_str());
    LOG_INFO("PREDICT", "  X - None of these");

    char choice = readCharFromUser();
    switch (choice) {
        case '1': return first;
        case '2': return second;
        case '3': return third;
        default:  return LABELS_COUNT;
    }
}

bool promptUserRetryPredictionSerial() {
    LOG_INFO("PREDICT", "[USER] > Retry prediction? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

bool promptUserForClearingDataAfterManyPredectionFailureSerial() {
    LOG_INFO("PREDICT", "[USER] > Delete data for all labels after many failures? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y'; 
}

// ======================== UTILS ========================

char readCharFromUserSerial() {
    char input[8];
    fgets(input, sizeof(input), stdin);
    return input[0];
}