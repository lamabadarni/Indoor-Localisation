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
    LOG_INFO("USER", "[USER] > Select log level:");
    LOG_INFO("USER", "  I - ERROR and INFO logs");
    LOG_INFO("USER", "  D - ERROR, INFO, and DEBUG logs");

    char choosen = readCharFromUserSerial();
    delay_ms(USER_PROMPTION_DELAY);
    if (choosen == 'i' || choosen == 'I') {
        SystemSetup::logLevel = LogLevel::LOG_LEVEL_INFO;
    } else if (choosen == 'd' || choosen == 'D') {
        SystemSetup::logLevel = LogLevel::LOG_LEVEL_DEBUG;
    }

    if(SystemSetup::logLevel == LogLevel::LOG_LEVEL_DEBUG) {
        LOG_INFO("USER", "[USER] ERROR, INFO, and DEBUG Log Level was choosen");
    }
    else if(SystemSetup::logLevel == LogLevel::LOG_LEVEL_INFO) {
        LOG_INFO("USER", "[USER] ERROR and INFO Log Level was choosen");
    }
}

static SystemMode promptUserSystemMode() {
    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("USER", "[USER] > Select System Mode:");
    for (int i = 0; i < MODES_NUM; ++i) {
        LOG_INFO("USER", "  %d - %s", i + 1, systemModes[i].c_str());
    }

    int sel = -1;
    while (sel < 1 || sel > MODES_NUM) sel = readIntFromUserSerial();
    return static_cast<SystemMode>(sel - 1);
}

static SystemBootMode promptUserSystemBootMode() {
    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("USER", "[USER] > Select System Boot Mode:");
    for (int i = 0; i < SYSTEM_BOOT_MODES_NUM; ++i) {
        LOG_INFO("USER", "  %d - %s", i + 1, systemBootModes[i].c_str());
    }

    int sel = -1;
    while (sel < 1 || sel > SYSTEM_BOOT_MODES_NUM) sel = readIntFromUserSerial();
    LOG_INFO("USER", "System Boot Mode : %s" , systemBootModes[sel-1].c_str());
    return static_cast<SystemBootMode>(sel - 1);
}

static SystemScannerMode promptUserScannerMode() {
    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("USER", "[USER] > Select Scanner Mode:");
    for (int i = 0; i < SYSTEM_SCANNER_MODES_NUM; ++i) {
        LOG_INFO("USER", "  %d - %s", i + 1, systemScannerModes[i].c_str());
    }

    int sel = -1;
    while (sel < 1 || sel > SYSTEM_SCANNER_MODES_NUM) sel = readIntFromUserSerial();
    return static_cast<SystemScannerMode>(sel - 1);
}
static SystemPredictionMode promptUserSystemPredictionMode() {
    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("USER", "[USER] > Select System Boot Mode:");
        for (int i = 0; i < SYSTEM_PREDICTION_NODES_NUM; ++i) {
        LOG_INFO("USER", "  %d - %s", i + 1, systemPredictionModes[i].c_str());
    }

    int sel = -1;
    while (sel < 1 || sel > SYSTEM_PREDICTION_NODES_NUM) sel = readIntFromUserSerial();
    return static_cast<SystemPredictionMode>(sel - 1);
}

void runUserSystemSetupSerial() {
    promptUserLoggerConfiguration();
    SystemSetup::currentSystemMode = promptUserSystemMode();
    LOG_INFO("USER", "[USER] System Mode : %s" , systemModes[SystemSetup::currentSystemMode].c_str());

    if (SystemSetup::currentSystemMode == MODE_SYSTEM_BOOT) {
        SystemSetup::currentSystemBootMode = promptUserSystemBootMode();
        LOG_INFO("USER", "[USER] System Boot Mode : %s" , systemBootModes[SystemSetup::currentSystemBootMode].c_str());
        return;
    }

    if( SystemSetup::currentSystemMode == MODE_SCANNING_SESSION || 
        SystemSetup::currentSystemMode == MODE_FULL_SESSION ) {
            SystemSetup::currentSystemScannerMode = promptUserScannerMode();
            SystemSetup::currentSystemPredictionMode = promptUserSystemPredictionMode();
            LOG_INFO("USER", "[USER] System Scanner Mode : %s" , systemScannerModes[SystemSetup::currentSystemScannerMode].c_str());
    }

    if (SystemSetup::currentSystemMode == MODE_PREDICTION_SESSION ||
        SystemSetup::currentSystemMode == MODE_FULL_SESSION) {
            SystemSetup::currentSystemPredictionMode = promptUserSystemPredictionMode();
            LOG_INFO("USER", "[USER] System Prediction Mode : %s" , systemPredictionModes[SystemSetup::currentSystemPredictionMode].c_str());

    }

    LOG_INFO("USER", "Enable validation phase? (y/n): ");
    char c = readCharFromUserSerial();
    if(c == 'y' || c == 'Y') SystemSetup::enableValidationPhase = true;
    return;
}

void promptUserShowDebugLogsSerial() {
    LOG_INFO("USER", "[USER] > Show debug logs during rescan? (y/n): ");
    char c = readCharFromUserSerial();
    if (c == 'y' || c == 'Y') {
        SystemSetup::logLevel = LogLevel::LOG_LEVEL_DEBUG;
    }
}

bool promptUserRunAnotherSessionSerial() {
    LOG_INFO("USER", "Would you like to run another session?");
    LOG_INFO("USER", "(y - yes | n - no)");        
    char again = readCharFromUserSerial();
    if (again == 'y' || again == 'Y') {
        return true;
    }
    return false;
}

// =======================================================
// 🟩 LABEL SELECTION
// =======================================================

void promptUserLocationLabelForScanSerial() {
    LOG_INFO("USER", "[USER] > Select label by index:");
    for (int i = 0; i < LABELS_COUNT; ++i) {
        LOG_INFO("USER", "  %d - %s", i + 1, labels[i].c_str());
    }

    int sel = -1;
    while (sel < 1 || sel > LABELS_COUNT) {
        sel = readIntFromUserSerial();        
    }
    
    currentLabel = static_cast<Label>(sel - 1);
}

void promptLabelsValidPredectionSerial() {
    //LAMA TODO
    LOG_INFO("USER", "[USER] Labels valid for prediction:");
    for (int i = 0; i < LABELS_COUNT; ++i) {
        LOG_INFO("USER", "  %d - %s", i + 1, labels[i].c_str());
    }
}

void promptUserProceedToNextLabelSerial() {
    LOG_INFO("USER", "[USER] > Proceed to another label? (y/n): ");
    char c = readCharFromUserSerial();
    shouldAbort = (c == 'N' || c == 'n');
}

// =======================================================
// 🟨 BACKUP & REUSE
// =======================================================

bool promptUserReuseDecisionSerial() {
    LOG_INFO("USER", "[USER] > Reuse saved scan?");
    LOG_INFO("USER", "  Y - Yes, reuse");
    LOG_INFO("USER", "  N - No, rescan");
    char c = readCharFromUserSerial();
    if (c == 'y' || c == 'Y') return true;
    return false;
}

// =======================================================
// 🟧 COVERAGE DIAGNOSTICS
// =======================================================

char promptUserRunCoverageDiagnosticSerial() {
    LOG_INFO("USER", "[USER] > Run scan coverage diagnostic before scanning?");
    LOG_INFO("USER", "  Y - Yes");
    LOG_INFO("USER", "  N - No");
    LOG_INFO("USER", "  D - Don't ask again");
    return readCharFromUserSerial();
}

bool promptUserAbortToImproveEnvironmentSerial() {
    LOG_INFO("USER", "[USER] > Abort to improve environment? (y/n): ");
    char c = readCharFromUserSerial();
    return c == 'y' || c == 'Y';
}

// =======================================================
// 🟥 VALIDATION PHASE
// =======================================================

bool promptUserRetryValidationSerial() {
    LOG_INFO("USER", "[USER] > Retry validation? (y/n): ");
    char c = readCharFromUserSerial();
    return c == 'y' || c == 'Y' ;
}

bool promptUserRescanAfterInvalidationSerial() {
    LOG_INFO("USER", "[USER] > Rescan after failed validation? (y/n): ");
    char c = readCharFromUserSerial();
    return c == 'y' || c == 'Y';
}

// =======================================================
// 🟪 PREDICTION PHASE
// =======================================================

Label promptUserChooseBetweenPredictionsSerial(Label left, Label right) {
    LOG_INFO("USER", "[USER] > Choose the correct prediction:");
    LOG_INFO("USER", "  1 -  (%s)", labels[left].c_str());
    LOG_INFO("USER", "  2 -  (%s)", labels[right].c_str());
    LOG_INFO("USER", "  X - Both predictions are invalid");

    int sel = readIntFromUserSerial();
    if (sel == 1) return left;
    if (sel == 2) return right;
    return LABELS_COUNT;
}

Label promptUserChooseBetweenTriplePredictionsSerial(Label first, Label second, Label third) {
    LOG_INFO("USER", "[USER] > Choose the correct prediction:");
    LOG_INFO("USER", "  1 - (%s)", labels[first].c_str());
    LOG_INFO("USER", "  2 - (%s)", labels[second].c_str());
    LOG_INFO("USER", "  3 - (%s)", labels[third].c_str());
    LOG_INFO("USER", "  X - None of these");

    char choice = readCharFromUserSerial();
    switch (choice) {
        case '1': return first;
        case '2': return second;
        case '3': return third;
        default:  return LABELS_COUNT;
    }
}

bool promptUserRetryPredictionSerial() {
    LOG_INFO("USER", "[USER] > Retry prediction? (y/n): ");
    char c = readCharFromUserSerial();
    return c == 'y' || c == 'Y';
}

bool promptUserForClearingDataAfterManyPredectionFailureSerial() {
    LOG_INFO("USER", "[USER] > Delete data for all labels after many failures? (y/n): ");
    char c = readCharFromUserSerial();
    return c == 'y' || c == 'Y'; 
}

// ======================== UTILS ========================

char readCharFromUserSerial() {
    int c;
    while (true) {
        c = getchar();
        if (c == '\r' || c == '\n') continue;
        if (c == 'r' || c == 'R') continue; // skip reset chars
        if (c == EOF) continue;
        return (char)c;
    }
}

int readIntFromUserSerial() {
    char buffer[16] = {0};
    int idx = 0;
    int c;

    while (true) {
        c = getchar();

        if (c == '\r' || c == '\n') {
            // Newline or carriage return -> end of input
            break;
        }

        if (c >= '0' && c <= '9' && idx < 15) {
            buffer[idx++] = (char)c;
            putchar(c);  // Echo back the digit
        }
    }

    buffer[idx] = '\0';  // Null-terminate the string
    putchar('\n');       // Move to next line on terminal
    return (idx > 0) ? atoi(buffer) : 0;
}