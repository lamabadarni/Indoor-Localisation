/**
 * @file scanning.cpp
 * @brief Manages full scanning sessions across labels, handles retries, validation, and measurement dispatching.
 * @author Lama Badarni
 */

#include "scanningPhase.h"
#include "rssiScanner.h"
#include "tofScanner.h"
#include "../ui/userUI.h"
#include "../validation/validationPhase.h"
#inclue "../utils/utilities.h"

static bool dontAskAgain = false;

void runScanningPhase() {
    delay_ms(DELAY_BETWEEN_PHASES);

    LOG_INFO("SCAN", "=============== Scanning Phase Started ===============");
    LOG_INFO("SCAN", "In this phase, the system will collect signal data from your environment.");
    LOG_INFO("SCAN", "Please stand still at the selected location while the scan is in progress.");
    LOG_INFO("SCAN", "Collected data will be used for training or prediction depending on the system mode.");

    for (int i = 0; i < NUMBER_OF_LABELS; i++) {
        delay_ms(DELAY_BETWEEN_PHASES);

        currentLabel = promptLocationLabel();
        if(reuseFromSD[currentLabel]) {
            int labelAccuracy;
            LOG_INFO("SCANNING", "Backup data available for label: %s", labels[currentLabel]);
            LOG_INFO("SCANNING", "Stored prediction accuracy: %d %", labelAccuracy);
            char input = prompUserReuseApprove();
            if(input == 'N' || input == 'n') break;
            if(input == 'V' || input == 'v') validateScanAccuracy();
        }

        Serial.println();
        LOG_INFO("SCANNING", "[SCAN] Selected Label: " + labels[currentLabel]);
        LOG_INFO("SCANNING", "[SCAN] Press Enter to start scanning...");
        readCharFromUser();

        startLabelScanningSession();
    }

    LOG_INFO("SCANNING", "=============== Scanning Phase Completed ===============");
}

bool startLabelScanningSession() {
    int retryCount = 0;
    bool validScan = false;
    if(!dontAskAgain) {
        char input = promptUserCoverageDiagnosticBeforeScan();
        if(input == 'Y' || input == 'y') performRSSIScanCoverage();
        if(input == 'D' || input == 'd') dontAskAgain = true;
    }

    while (retryCount < MAX_RETRIES_FOR_RSSI) {
        LOG_INFO("SCANNING", "[SCAN] Attempt # %d for: %s",  String(retryCount + 1), labels[currentLabel]));

        collectMeasurements();

        LOG_INFO("SCANNING", "[SCAN] Validating scan accuracy...");

        validScan = validateScanAccuracy();
        if (validScan) {
            LOG_INFO("SCANNING", " Scan successful at: %s ", labels[currentLabel]);
            break;
        }

        LOG_INFO("SCANNING", "[SCAN] Accuracy insufficient for user, retrying...");
        retryCount++;
    }

    if (!validScan) {
        Serial.println("[SCAN] Scan failed after max retries at: " + String(labelToString(currentLabel)));
        return false;
    }

    Serial.println("[SCAN] Final accepted scan at: " + String(labelToString(currentLabel)) + ", Accuracy: " + String(accuracy) + "%");

    return true;
}

void collectMeasurements() {
    Serial.println("[SCAN] Collecting measurements...");

    switch (currentSystemState) {
        case STATIC_RSSI:
            LOG_INFO("SCANNING", "[SCAN] Mode: STATIC_RSSI");
            performRSSIScan();
            break;

        case STATIC_RSSI_TOF:
            LOG_INFO("SCANNING", "[SCAN] Mode: STATIC_RSSI_TOF");
            performRSSIScan();
            //performTOFScan();
            break;

        default:
            LOG_INFO("SCANNING", "[SCAN] Unsupported system state.");
            break;
    }
}

void scanStaticRSSI() {
    performRSSIScan();
}

void scanTOF() {
    LOG_INFO("SCANNING", "[SCAN] FTM not implemented.");
}

void scanDynamicRSSI() {
   LOG_INFO("SCANNING", "[SCAN] Dynamic RSSI scanning not implemented.");
}