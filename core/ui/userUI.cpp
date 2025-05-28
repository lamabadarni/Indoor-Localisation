#include "../utils/platform.h"
#include "../utils/utilities.h"
#include "../ui/logger.h"
#include "userUI.h"

// ========== SYSTEM SETUP ==========
static LogLevel promptUserLoggerConfiguration() {
    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("SETUP", "[USER] > Choose log level ");
    LOG_INFO("SETUP", "E - ERROR logs ");
    LOG_INFO("SETUP", "W - ERROR and WARN log ");
    LOG_INFO("SETUP", "I - ERROR , WARN and INFO logs ");
    LOG_INFO("SETUP", "D - ERROR , WARN , INFO and DEBUG logs ");

    char choosen = readCharFromUser();
    delay_ms(USER_PROMPTION_DELAY);
    if(choosen == 'e' || choosen == 'E') {
        SystemSetup::logLevel = LogLevel::LOG_LEVEL_ERROR;
    }
    else if(choosen == 'w' || choosen == 'W') {
        SystemSetup::logLevel = LogLevel::LOG_LEVEL_WARN;
    }
    else if(choosen == 'i' || choosen == 'I') {
        SystemSetup::logLevel = LogLevel::LOG_LEVEL_INFO;
    }
    else if(choosen == 'd' || choosen == 'D') {
        SystemSetup::logLevel = LogLevel::LOG_LEVEL_DEBUG;
    }
}

static SystemMode promptUserSystemMode() {
    delay_ms(USER_PROMPTION_DELAY);
    LOG_INFO("SETUP", "[USER] > Select System Mode:");
    for (int i = 0; i < MODES_NUM; ++i) {
        LOG_INFO("SETUP", "  %d - %s", i + 1, systemModes[i].c_str());
    }
    delay_ms(USER_PROMPTION_DELAY);
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

    if(SystemSetup::currentSystemMode == MODE_SYSTEM_BOOT) {
        return;
    }

    SystemSetup::currentSystemScannerMode = promptUserScannerMode();

    struct {
        const char* prompt;
        bool* flag;
    } toggles[] = {
        {"Enable backup?           (y/n): "  , &SystemSetup::enableBackup},
        {"Enable validation phase? (y/n): "  , &SystemSetup::enableValidationPhase},
        {"Enable restore?          (y/n): "  , &SystemSetup::enableRestore}
    };

    for (auto& opt : toggles) {
        LOG_INFO("SETUP", "[USER] > %s", opt.prompt);
        *opt.flag = (readCharFromUser() == 'y' || readCharFromUser() == 'Y');
    }
}

// ========== LABEL ==========
void promptUserLocationLabel() {
    LOG_INFO("LABEL", "[USER] > Select label by index:");
    for (int i = 0; i < LABELS_COUNT; ++i) {
        LOG_INFO("LABEL", "  %d - %s", i + 1, labels[i].c_str());
    }
    int sel = -1;
    while (sel < 1 || sel > LABELS_COUNT) sel = readIntFromUser();
    currentLabel = static_cast<Label>(sel - 1);
}

// ========== FEEDBACK ==========
bool promptUserApproveScanAccuracy() {
    LOG_INFO("FEEDBACK", "[USER] > Approve scan accuracy? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

bool promptUserRescanAfterInvalidation() {
    LOG_INFO("FEEDBACK", "[USER] > Rescan after failed validation? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

bool promptUserRetryValidation() {
    LOG_INFO("FEEDBACK", "[USER] > Retry prediction? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

bool promptUserCoverageSufficient() {
    LOG_INFO("FEEDBACK", "[USER] > Is coverage sufficient? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

bool promptUserProceedToNextLabel() {
    LOG_INFO("FEEDBACK", "[USER] > Proceed to next label? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

bool promptUserAbortToImproveEnvironment() {
    LOG_INFO("FEEDBACK", "[USER] > Abort to improve environment? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

void promptUserShowDebugLogs() {
    LOG_INFO("FEEDBACK", "[USER] > Show debug logs during rescan? (y/n): ");
    char c = readCharFromUser();
    SystemSetup::logLevel = (c == 'y' || c == 'Y') ? LogLevel::LOG_LEVEL_DEBUG : SystemSetup::logLevel;
}

// ========== PREDICTION ==========
bool promptUserApprovePrediction() {
    LOG_INFO("PREDICT", "[USER] > Approve predicted label? (y/n): ");
    char c = readCharFromUser();
    return c == 'y' || c == 'Y';
}

Label promptUserChooseBetweenPredictions(Label rssi, Label tof) {
    LOG_INFO("PREDICT", "[USER] > Choose between predictions:");
    LOG_INFO("PREDICT", "1 - RSSI (%s)", labels[rssi].c_str());
    LOG_INFO("PREDICT", "2 - TOF  (%s)", labels[tof].c_str());
    int sel = readIntFromUser();
    return (sel == 2) ? tof : rssi;
}

// ========== REUSE / ABORT ==========
char promptUserReuseDecision() {
    LOG_INFO("UI", "[USER] > Reuse saved scan?");
    LOG_INFO("UI", "Y - Yes, reuse");
    LOG_INFO("UI", "V - Validate again");
    LOG_INFO("UI", "N - No, rescan");
    return readCharFromUser();
}

char promptUserRunCoverageDiagnostic() {
    LOG_INFO("UI", "[USER] > Run RSSI coverage diagnostic?");
    LOG_INFO("UI", "Y - Yes");
    LOG_INFO("UI", "N - No");
    LOG_INFO("UI", "D - Don't ask again");
    return readCharFromUser();
}

void promptUserAbortOrContinue(bool allowAbort) {
    LOG_INFO("SYSTEM", "[USER] > Press 'c' to continue...");

    char input = getCharFromUserWithTimeout(10000);
    if (input == 'c' || input == 'C') {
        LOG_INFO("SYSTEM", "Continuing...");
        return;
    }


    LOG_INFO("SYSTEM", "No input detected. Abort current phase?");
    LOG_INFO("SYSTEM", "c - Continue, q - Abort full system");
    input = getCharFromUserWithTimeout(15000);
    if (input == 'q' || input == 'Q') {
        LOG_INFO("SYSTEM", "User aborted full session.");
        shouldAbort = true;
    }
}
