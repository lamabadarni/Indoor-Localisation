/**
 * @file userUI.cpp
 * @brief Handles all user prompts for system configuration, scanning, validation, prediction, and abort flow.
 *
 * Each function is responsible for a specific interactive prompt and updates global config or state accordingly.
 * 
 */

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "userUI.h"

// =======================================================
// 🟦 SYSTEM SETUP PROMPTS
// =======================================================

static void promptUserLoggerConfiguration() {
    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("SETUP", "[USER] > Choose log level:");
    LOG_INFO("SETUP", "  I - ERROR, WARN, and INFO logs");
    LOG_INFO("SETUP", "  D - ERROR, WARN, INFO, and DEBUG logs");

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

void runUserSystemSetup() {
    promptUserLoggerConfiguration();
    SystemSetup::currentSystemMode = promptUserSystemMode();

    if (SystemSetup::currentSystemMode == MODE_SYSTEM_BOOT) return;

    SystemSetup::currentSystemScannerMode = promptUserScannerMode();

    struct {
        const char* prompt;
        bool* flag;
    } toggles[] = {
        {"Enable backup?           (y/n): ", &SystemSetup::enableBackup},
        {"Enable validation phase? (y/n): ", &SystemSetup::enableValidationPhase},
        {"Enable restore?          (y/n): ", &SystemSetup::enableRestore}
    };

    for (auto& opt : toggles) {
        LOG_INFO("SETUP", "[USER] > %s", opt.prompt);
        *opt.flag = (readCharFromUser() == 'y' || readCharFromUser() == 'Y');
    }
}

void promptUserShowDebugLogs() {
    LOG_INFO("FEEDBACK", "[USER] > Show debug logs during rescan? (y/n): ");
    char c = readCharFromUser();
    if (c == 'y' || c == 'Y') {
        SystemSetup::logLevel = LogLevel::LOG_LEVEL_DEBUG;
    }
}

// =======================================================
// 🟩 LABEL SELECTION
// =======================================================

void promptUserLocationLabel() {
    LOG_INFO("LABEL", "[USER] > Select label by index:");
    for (int i = 0; i < LABELS_COUNT; ++i) {
        for(auto label : skippedLabels) {
            if(label == i) {
                LOG_INFO("LABEL", "  %d - %s was skipped", i + 1, labels[i].c_str());
                continue;
            }
        }
        LOG_INFO("LABEL", "  %d - %s", i + 1, labels[i].c_str());
    }

    int sel = -1;
    while (sel < 1 || sel > LABELS_COUNT) {
        sel = readIntFromUser();
        for(auto label : skippedLabels) {
            if(label == sel) {
                //TODO: should approve this choice
            }
        currentLabel = static_cast<Label>(sel - 1);
        }
    }
}

void promptLabelsValidToPredection() {
    LOG_INFO("LABEL", "[USER] Labels valid for prediction:");
    for (int i = 0; i < LABELS_COUNT; ++i) {
        LOG_INFO("LABEL", "  %d - %s", i + 1, labels[i].c_str());
    }
}

bool promptUserProceedToNextLabel() {
    LOG_INFO("LABEL", "[USER] > Proceed to another label? (y/n): ");
    char c = readCharFromUser();
    if (c == 'y' || c == 'Y') return true;

    promptUserAbortOrContinue();
    return !shouldAbort;
}

// =======================================================
// 🟨 BACKUP & REUSE
// =======================================================

char promptUserReuseDecision() {
    LOG_INFO("BACKUP", "[USER] > Reuse saved scan?");
    LOG_INFO("UI", "  Y - Yes, reuse");
    LOG_INFO("UI", "  V - Validate first");
    LOG_INFO("UI", "  N - No, rescan");
    return readCharFromUser();
}

// =======================================================
// 🟧 COVERAGE DIAGNOSTICS
// =======================================================

char promptUserRunCoverageDiagnostic() {
    LOG_INFO("SCAN", "[USER] > Run scan coverage diagnostic before scanning?");
    LOG_INFO("UI", "  Y - Yes");
    LOG_INFO("UI", "  N - No");
    LOG_INFO("UI", "  D - Don't ask again");
    return readCharFromUser();
}

bool promptUserAbortToImproveEnvironment() {
    LOG_INFO("COVERAGE", "[USER] > Abort to improve environment? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

// =======================================================
// 🟥 VALIDATION PHASE
// =======================================================

bool promptUserRescanAfterInvalidation() {
    LOG_INFO("VALIDATE", "[USER] > Rescan after failed validation? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

bool promptUserForClearingDataAfterManyPredectionFailure() {
    //TODO: ask user if to delete the data
    return true;
}

// =======================================================
// 🟪 PREDICTION PHASE
// =======================================================

bool promptUserApprovePrediction() {
    LOG_INFO("PREDICT", "[USER] > Approve predicted label? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

Label promptUserChooseBetweenPredictions(Label rssi, Label tof) {
    LOG_INFO("PREDICT", "[USER] > Choose between predictions:");
    LOG_INFO("PREDICT", "  1 - RSSI (%s)", labels[rssi].c_str());
    LOG_INFO("PREDICT", "  2 - TOF  (%s)", labels[tof].c_str());
    LOG_INFO("PREDICT", "  X - Both predictions are invalid");

    int sel = readIntFromUser();
    if (sel == 1) return rssi;
    if (sel == 2) return tof;
    return LABELS_COUNT;
}

bool promptUserRetryPrediction() {
    LOG_INFO("PREDICT", "[USER] > Retry prediction? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

// =======================================================
// 🛑 ABORT FLOW
// =======================================================

void promptUserAbortOrContinue() {
    LOG_INFO("SYSTEM", "[USER] > Press 'c' to continue...");

    char input = getCharFromUserWithTimeout(10000);
    if (input == 'c' || input == 'C') {
        LOG_INFO("SYSTEM", "Continuing...");
        return;
    }

    LOG_INFO("SYSTEM", "No input detected. Abort current phase?");
    LOG_INFO("SYSTEM", "  c - Continue");
    LOG_INFO("SYSTEM", "  q - Abort full system");

    input = getCharFromUserWithTimeout(15000);
    if (input == 'q' || input == 'Q') {
        LOG_INFO("SYSTEM", "User aborted full session.");
        shouldAbort = true;
    }

    LOG_INFO("SYSTEM", "No input detected. Saving data and returning to main menu.");
}
