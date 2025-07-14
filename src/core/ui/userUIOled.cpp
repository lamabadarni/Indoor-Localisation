/**
 * @file userUI.cpp
 * @brief Handles all user prompts for system configuration, scanning, validation, prediction, and abort flow.
 *
 * Each function logs its prompt to the serial console and simultaneously displays an interactive prompt on the screen.
 */

#include "userUIOled.h"
#include "core/ui/display.h"      // The display manager
#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include <vector>
#include <string>

/*
// =======================================================
// 🟦 SYSTEM SETUP PROMPTS
// =======================================================

static void promptUserLoggerConfiguration() {
    delay_ms(USER_PROMPTION_DELAY);

    std::vector<std::string> items = {"Info Logs", "Debug Logs"};
    int choice = display_prompt_menu("Select Log Level", items);
    if (choice == 0) {
            SystemSetup::logLevel = LOG_LEVEL_INFO ;
                LOG_INFO("SETUP", "USER> Choose info level:");
    }
    else {
            SystemSetup::logLevel = LOG_LEVEL_DEBUG;
                LOG_INFO("SETUP", "[USER]  Choose log level:");
    }
}

static SystemMode promptUserSystemMode() {
    delay_ms(USER_PROMPTION_DELAY);
    int sel = display_prompt_menu("Select System Mode", arrayToVector(systemModes, MODES_NUM));
    for (int i = 0; i < MODES_NUM; ++i) {
        LOG_INFO("SETUP", "  %d - %s", i + 1, systemModes[i].c_str());
    }
    return static_cast<SystemMode>(sel);
}
/// @brief todo add prompt user system boot mode!!!!!!
/// @return todo prompt user pridictation monde
static SystemBootMode promptUserSystemBootMode() {
    delay_ms(USER_PROMPTION_DELAY);
    int sel = display_prompt_menu("Select System Boot Mode", arrayToVector(systemBootModes, SYSTEM_BOOT_MODES_NUM));
    LOG_INFO("SETUP", "[USER] > Select System Boot Mode:");
    for (int i = 0; i < SYSTEM_BOOT_MODES_NUM; ++i) {
        LOG_INFO("SETUP", "  %d - %s", i + 1, systemBootModes[i].c_str());
    }

    return static_cast<SystemBootMode>(sel);
}



static SystemPredictionMode promptUserSystemPredictionMode() {
    delay_ms(USER_PROMPTION_DELAY);
    int sel = display_prompt_menu("Select System Boot Mode", arrayToVector(systemPredictionModes, SYSTEM_BOOT_MODES_NUM));
    LOG_INFO("SETUP", "[USER] > Select System Boot Mode:");
    for (int i = 0; i < SYSTEM_PREDICTION_NODES_NUM; ++i) {
        LOG_INFO("SETUP", "  %d - %s", i + 1, systemPredictionModes[i].c_str());
    }
    return static_cast<SystemPredictionMode>(sel);
}

static SystemScannerMode promptUserScannerMode() {
    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("SETUP", "[USER] > Select Scanner Mode:");
    for (int i = 0; i < SYSTEM_SCANNER_MODES_NUM; ++i) {
        LOG_INFO("SETUP", "  %d - %s", i + 1, systemScannerModes[i].c_str());
    }

    int sel = display_prompt_menu("Select Scanner Mode", arrayToVector(systemScannerModes, SYSTEM_SCANNER_MODES_NUM));
    return static_cast<SystemScannerMode>(sel);
}

void runUserSystemSetupOLED() { //// check new changes 
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

    if( SystemSetup::currentSystemMode == MODE_PREDICTION_SESSION || 
        SystemSetup::currentSystemMode == MODE_FULL_SESSION ) {
            SystemSetup::currentSystemPredictionMode = promptUserSystemPredictionMode();
    }
    
    struct {
        const char* prompt;
        bool* flag;
    } toggles[] = {
        {"Enable validation phase? (y/n): ", &SystemSetup::enableValidationPhase},
    };

    for (auto& opt : toggles) {
        LOG_INFO("SETUP", "[USER] > %s", opt.prompt);
        *opt.flag = display_prompt_yes_no("Setup", "Enable validation phase? (y/n): ");
    }
}


void promptUserShowDebugLogsOLED() {
    LOG_INFO("FEEDBACK", "[USER] > Show debug logs during rescan? (y/n): ");
    if (display_prompt_yes_no("Debug", "Show debug logs?")) {
        SystemSetup::logLevel = LogLevel::LOG_LEVEL_DEBUG;
    }
}

// =======================================================
// 🟩 LABEL SELECTION
// =======================================================

void promptUserLocationLabelForScanOLED() { //// for scaaaan 
    LOG_INFO("LABEL", "[USER] > Select label by index:");
    // The complex skipping logic is preserved in the logs, but simplified for the display menu.
    // You can add more complex logic after the display selection if needed.
    for (int i = 0; i < LABELS_COUNT; ++i) {
        LOG_INFO("LABEL", "  %d - %s", i + 1, labels[i].c_str());
    }

    int sel = display_prompt_menu("Select Location Label", arrayToVector(labels, LABELS_COUNT));
    currentLabel = static_cast<Label>(sel);
}

void promptLabelsValidToPredectionOLED() { /// for predictation
    LOG_INFO("LABEL", "[USER] Labels valid for prediction:");
    for (int i = 0; i < LABELS_COUNT; ++i) {
        LOG_INFO("LABEL", "  %d - %s", i + 1, labels[i].c_str());
    }
    // Also show the list on the display. The user selects any item to dismiss.
    display_prompt_menu("Valid Labels (View Only)", arrayToVector(labels, LABELS_COUNT));
}

void promptUserProceedToNextLabelOLED() { //// make it void and fill should aboot according to user retuned yes
    LOG_INFO("LABEL", "[USER] > Proceed to another label? (y/n): ");
    bool proceed = display_prompt_yes_no("[USER]", "Proceed to another label? (y/n):");
    if (!proceed)
    shouldAbort=true;
}

// =======================================================
// 🟨 BACKUP & REUSE
// =======================================================

bool promptUserReuseDecisionOLED() { // convert to bool


    std::vector<std::string> items = {"Yes, reuse", "No, rescan"};
    int choice = display_prompt_menu("Reuse saved scan?", items);
    if (choice == 0){ 
      LOG_INFO("LABEL", "[USER] > user chosed to reuse !  ");
        return true;
    }
    LOG_INFO("LABEL", "[USER] > user chosed to rescan ! ");
     return false;
}
// =======================================================
// 🟧 COVERAGE DIAGNOSTICS
// =======================================================

char promptUserRunCoverageDiagnosticOLED() {
    
    std::vector<std::string> items = {"Yes", "No", "Don't ask again"};
    int choice = display_prompt_menu("Run coverage check?", items);
    if (choice == 0) return 'Y';
    if (choice == 1) return 'N';
    return 'D';
}

bool promptUserAbortToImproveEnvironmentOLED() {
    LOG_INFO("COVERAGE", "[USER] > Abort to improve environment? (y/n): ");
    return display_prompt_yes_no("Coverage Low", "Abort to improve?");
}

// =======================================================
// 🟥 VALIDATION PHASE
// =======================================================

bool promptUserRescanAfterInvalidationOLED() {
    return display_prompt_yes_no("Validation Failed", "Rescan location?");
}


// =======================================================
// 🟪 PREDICTION PHASE
// =======================================================

bool promptUserApprovePredictionOLED() {
    return display_prompt_yes_no("Prediction OK?", "Approve this label?");
}

Label promptUserChooseBetweenPredictionsOLED(Label left, Label right) { //TODO  check names


    std::vector<std::string> items;
    items.push_back("Left: " + labels[left]);
    items.push_back("Right: " + labels[right]);
    items.push_back("Both are Invalid");

    int sel = display_prompt_menu("Choose the correct prediction", items);
    if (sel == 0){ 
     LOG_INFO("PREDICT", "[USER] >  Choose the left prediction");
        return left;
    }
    if (sel == 1){ 
     LOG_INFO("PREDICT", "[USER] > Choose the right prediction");
        return right;
    }
     LOG_INFO("PREDICT", "[USER] > Didnt Choose any prediction");
    return LABELS_COUNT;
}


bool promptUserRetryPredictionOLED() {
    LOG_INFO("PREDICT", "[USER] > Retry prediction? (y/n): ");
    return display_prompt_yes_no("Prediction Failed", "Retry prediction?");
}

char readCharFromUserOLED(){
    return display_wait_for_any_button();
} 

bool promptUserRunAnotherSessionOLED(){
    int choice = display_prompt_yes_no("main","Would you like to run another session?");
     if (choice) { 
        LOG_INFO("Main", "[USER] > want to Start a another session ");
    } 
    else {
        LOG_INFO("Main", "[USER] > No need for another session");
    }

    return choice;

}

bool promptUserForClearingDataAfterManyPredectionFailureOLED() {
    bool choice = display_prompt_yes_no("PREDICT", " [USER] > Delete data for all labels after many failures? (y/n):"); // Pass the formatted string  
    return choice; 
}*/