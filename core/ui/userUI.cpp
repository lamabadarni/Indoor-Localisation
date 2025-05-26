#include "../utils/platform.h"
#include "../utils/utilities.h"
#include "../ui/logger.h"
#include "userUI.h"

// ========== SYSTEM SETUP ==========
static void promptUserLoggerConfiguration() {
    LOG_INFO("SETUP", "[USER] > Enable INFO logs? (y/n): ");
    SystemSetup::printInfoLogs = (readCharFromUser() == 'y' || readCharFromUser() == 'Y');

    LOG_INFO("SETUP", "[USER] > Enable DEBUG logs? (y/n): ");
    SystemSetup::printDebugLogs = (readCharFromUser() == 'y' || readCharFromUser() == 'Y');

    LOG_INFO("SETUP", "Logger configured: INFO=%d, DEBUG=%d", 
             SystemSetup::printInfoLogs, SystemSetup::printDebugLogs);
}

static SystemMode promptUserSystemMode() {
    LOG_INFO("SETUP", "[USER] > Select System Mode:");
    for (int i = 0; i < MODES_NUM; ++i) {
        LOG_INFO("SETUP", "  %d - %s", i + 1, systemModes[i].c_str());
    }
    int sel = -1;
    while (sel < 1 || sel > MODES_NUM) sel = readIntFromUser();
    return static_cast<SystemMode>(sel - 1);
}

static SystemScannerMode promptUserScannerMode() {
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
    SystemSetup::currentSystemScannerMode = promptUserScannerMode();

    struct {
        const char* prompt;
        bool* flag;
    } toggles[] = {
        {"Enable SD card backup? (y/n): ", &SystemSetup::forceSDCardBackup},
        {"Run validation phase? (y/n): ", &SystemSetup::validateWhileScanningPhase},
        {"Enable label selection? (y/n): ", &SystemSetup::chooseLabelsToScan}
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
    SystemSetup::printDebugLogs = (c == 'y' || c == 'Y');
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

    if (allowAbort) {
        LOG_INFO("SYSTEM", "No input detected. Abort current phase?");
        LOG_INFO("SYSTEM", "c - Continue, x - Abort phase, q - Abort full system");
        input = getCharFromUserWithTimeout(15000);
        if (input == 'x' || input == 'X') {
            LOG_INFO("SYSTEM", "User aborted this phase.");
            forceNextPhase = true;
        } else if (input == 'q' || input == 'Q') {
            LOG_INFO("SYSTEM", "User aborted full session.");
            shouldAbort = true;
        }
    }
}
