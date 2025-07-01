/**
 * @file userUI.cpp
 * @brief Handles all user prompts for system configuration, scanning, validation, prediction, and abort flow.
 *
 * Each function logs its prompt to the serial console and simultaneously displays an interactive prompt on the screen.
 */

#include "userUI.h"
#include "core/ui/display.h"      // The display manager
#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include <vector>
#include <string>


// =======================================================
// 🟦 SYSTEM SETUP PROMPTS
// =======================================================

static void promptUserLoggerConfiguration() {
    delay_ms(USER_PROMPTION_DELAY);

    std::vector<std::string> items = {"Info Logs", "Debug Logs"};
    int choice = display_prompt_menu("Select Log Level", items);
    if (choice == 0) {
            SystemSetup::logLevel = LOG_LEVEL_INFO ;
                LOG_INFO_OLED("SETUP", "USER> Choose info level:");
    }
    else {
            SystemSetup::logLevel = LOG_LEVEL_DEBUG;
                LOG_INFO_OLED("SETUP", "[USER]  Choose log level:");
    }
}

static SystemMode promptUserSystemMode() {
    delay_ms(USER_PROMPTION_DELAY);
    int sel = display_prompt_menu("Select System Mode", arrayToVector(systemModes, MODES_NUM));
    for (int i = 0; i < MODES_NUM; ++i) {
        LOG_INFO_OLED("SETUP", "  %d - %s", i + 1, systemModes[i].c_str());
    }
    return static_cast<SystemMode>(sel);
}

static SystemScannerMode promptUserScannerMode() {
    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO_OLED("SETUP", "[USER] > Select Scanner Mode:");
    for (int i = 0; i < SYSTEM_SCANNER_MODES_NUM; ++i) {
        LOG_INFO_OLED("SETUP", "  %d - %s", i + 1, systemScannerModes[i].c_str());
    }

    int sel = display_prompt_menu("Select Scanner Mode", arrayToVector(systemScannerModes, SYSTEM_SCANNER_MODES_NUM));
    return static_cast<SystemScannerMode>(sel);
}

void runUserSystemSetup() {
    promptUserLoggerConfiguration();
    SystemSetup::currentSystemMode = promptUserSystemMode();

    if (SystemSetup::currentSystemMode == MODE_SYSTEM_BOOT) return;

    SystemSetup::currentSystemScannerMode = promptUserScannerMode();

    struct {
        const char* prompt;
        const char* menu_question;
        bool* flag;
    } toggles[] = {
        {"Enable backup?           (y/n): ", "Enable Backup?", &SystemSetup::enableBackup},
        {"Enable validation phase? (y/n): ", "Enable Validation?", &SystemSetup::enableValidationPhase},
        {"Enable restore?          (y/n): ", "Enable Restore?", &SystemSetup::enableRestore}
    };

    for (auto& opt : toggles) {
        LOG_INFO_OLED("SETUP", "[USER] > %s", opt.prompt);
        *opt.flag = display_prompt_yes_no("Setup", opt.menu_question);
    }
}

void promptUserShowDebugLogs() {
    LOG_INFO_OLED("FEEDBACK", "[USER] > Show debug logs during rescan? (y/n): ");
    if (display_prompt_yes_no("Debug", "Show debug logs?")) {
        SystemSetup::logLevel = LogLevel::LOG_LEVEL_DEBUG;
    }
}

// =======================================================
// 🟩 LABEL SELECTION
// =======================================================

void promptUserLocationLabel() {
    LOG_INFO_OLED("LABEL", "[USER] > Select label by index:");
    // The complex skipping logic is preserved in the logs, but simplified for the display menu.
    // You can add more complex logic after the display selection if needed.
    for (int i = 0; i < LABELS_COUNT; ++i) {
        LOG_INFO_OLED("LABEL", "  %d - %s", i + 1, labels[i].c_str());
    }

    int sel = display_prompt_menu("Select Location Label", arrayToVector(labels, LABELS_COUNT));
    currentLabel = static_cast<Label>(sel);
}

void promptLabelsValidToPredection() {
    LOG_INFO_OLED("LABEL", "[USER] Labels valid for prediction:");
    for (int i = 0; i < LABELS_COUNT; ++i) {
        LOG_INFO_OLED("LABEL", "  %d - %s", i + 1, labels[i].c_str());
    }
    // Also show the list on the display. The user selects any item to dismiss.
    display_prompt_menu("Valid Labels (View Only)", arrayToVector(labels, LABELS_COUNT));
}

bool promptUserProceedToNextLabel() {
    LOG_INFO_OLED("LABEL", "[USER] > Proceed to another label? (y/n): ");
    bool proceed = display_prompt_yes_no("Continue", "Scan another label?");
    if (!proceed) {
        promptUserAbortOrContinue(); // Call the abort flow if user says no
    }
    return proceed && !shouldAbort;
}

// =======================================================
// 🟨 BACKUP & REUSE
// =======================================================

char promptUserReuseDecision() {


    std::vector<std::string> items = {"Yes, reuse", "Validate first", "No, rescan"};
    int choice = display_prompt_menu("Reuse saved scan?", items);
    if (choice == 0) return 'Y';
    if (choice == 1) return 'V';
    return 'N';
}

// =======================================================
// 🟧 COVERAGE DIAGNOSTICS
// =======================================================

char promptUserRunCoverageDiagnostic() {
    
    std::vector<std::string> items = {"Yes", "No", "Don't ask again"};
    int choice = display_prompt_menu("Run coverage check?", items);
    if (choice == 0) return 'Y';
    if (choice == 1) return 'N';
    return 'D';
}

bool promptUserAbortToImproveEnvironment() {
    LOG_INFO("COVERAGE", "[USER] > Abort to improve environment? (y/n): ");
    return display_prompt_yes_no("Coverage Low", "Abort to improve?");
}

// =======================================================
// 🟥 VALIDATION PHASE
// =======================================================

bool promptUserRescanAfterInvalidation() {
    return display_prompt_yes_no("Validation Failed", "Rescan location?");
}

bool promptUserForClearingDataAfterManyPredectionFailure() {
    //TODO: ask user if to delete the data
    return display_prompt_yes_no("Error", "Clear all data?");
}

// =======================================================
// 🟪 PREDICTION PHASE
// =======================================================

bool promptUserApprovePrediction() {
    return display_prompt_yes_no("Prediction OK?", "Approve this label?");
}

Label promptUserChooseBetweenPredictions(Label rssi, Label tof) {


    std::vector<std::string> items;
    items.push_back("RSSI: " + labels[rssi]);
    items.push_back("TOF: " + labels[tof]);
    items.push_back("Both are Invalid");

    int sel = display_prompt_menu("Choose Prediction", items);
    if (sel == 0) return rssi;
    if (sel == 1) return tof;
    return LABELS_COUNT;
}

bool promptUserRetryPrediction() {
    LOG_INFO("PREDICT", "[USER] > Retry prediction? (y/n): ");
    return display_prompt_yes_no("Prediction Failed", "Retry prediction?");
}

// =======================================================
// 🛑 ABORT FLOW
// =======================================================

void promptUserAbortOrContinue() {

    // On a display, we ask a direct question instead of waiting for a timeout.
    if (!display_prompt_yes_no("Abort?", "Quit the session?")) {
        LOG_INFO_OLED("SYSTEM", "User chose to continue.");
        shouldAbort = false;
    } else {
        LOG_INFO_OLED("SYSTEM", "User chose to abort full session.");
        shouldAbort = true;
    }
}