/**
 * @file scanningPhase.cpp
 * @brief Implements scanning flow control, validation retries, and mode-specific measurement collection.
 *
 * Encapsulates the complete scanning logic across all user-defined labels,
 * including reuse handling, backup validation, and automatic retries on failure.
 * 
 */

#include "scanningPhase.h"
#include "staticRSSIScanner.h"
#include "tofScanner.h"
#include "dynamicRSSIScanner.h"
#include "core/validation/validationPhase.h"
#include "core/systemBootModeHandlers/diagnostics.h"

static bool dontAskAgain = false;

// =======================================================
// MAIN LABEL SCAN PHASE
// =======================================================

void runScanningPhase() {
    delay_ms(DELAY_BETWEEN_PHASES);

    LOG_INFO("SCAN", "=============== Scanning Phase Started ===============");
    LOG_INFO("SCAN", "In this phase, the system will collect data for each defined location label.");
    LOG_INFO("SCAN", "Please stand still at the selected location during scanning.");
    LOG_INFO("SCAN", "You will be prompted to choose whether to reuse existing data or perform a new scan.");
    LOG_INFO("SCAN", "Scanning will continue until all labels are processed or you decide to abort.");

    bool reuse = false; 
    bool labelInvalid = false;

    if (isDataLoaded()) {
        LOG_INFO("SCAN", "Backup data available");
        reuse = promptUserReuseDecision();
    }


    for (int i = 0; i < LABELS_COUNT; ++i) {
        delay_ms(DELAY_BETWEEN_PHASES);
        promptUserLocationLabel();  // Sets currentLabel
        LOG_INFO("SCAN", "Selected label: %s", labels[currentLabel].c_str());

        // Backup data validation path
        if (reuse) {
            LOG_INFO("SCAN", "Checking backup data accuracy for label %s", labels[currentLabel]);
            startLabelValidationSession();
            if (validForPredection[currentLabel]) continue; // Reuse backup if valid
            else LOG_INFO("SCAN", "Data seems to be not accurate for %s | accuracy : %d . Rescanning", labels[currentLabel], getAccuracyForValidation());
        }

        // Fresh scan path
        LOG_INFO("SCAN", "Press any key to start scanning...");
        readCharFromUser();

        if (!startLabelScanningSession()) {
            LOG_INFO("SCAN", "Skipping label: %s due to repeated validation failure.", labels[currentLabel].c_str());
            validForPredection[currentLabel] = false;
            labelInvalid = true;
        }
    }

    LOG_INFO("SCAN", "=============== Scanning Phase Completed ===============");

    if (labelInvalid) {
        LOG_INFO("SCAN", "The following labels were skipped due to scan failure:");
        for (int i = 0; i < LABELS_COUNT; i++) {
            if(!validForPredection[i]) {
            LOG_INFO("SCAN", " - %s | Accuracy: %.2f%%", labels[i].c_str(), getAccuracyForValidation());
            }
        }
    } else {
        LOG_INFO("SCAN", "All labels were scanned successfully.");
    }
}

// =======================================================
// SINGLE LABEL SESSION W/ RETRIES
// =======================================================

bool startLabelScanningSession() {
    int  retryCount = 0;
    bool validScan = false;

    if (!dontAskAgain) {
        char input = promptUserRunCoverageDiagnostic();
        if (input == 'Y' || input == 'y') performRSSIScanCoverage();
        if (input == 'D' || input == 'd') dontAskAgain = true;
    }

    while (retryCount < MAX_RETRIES_FOR_RSSI) {
        LOG_INFO("SCAN", "Collecting data attempt #%d for label: %s", retryCount + 1, labels[currentLabel].c_str());

        collectMeasurements();

        LOG_INFO("SCAN", "Validating data collected accuracy...");
        startLabelValidationSession();

        if (validScan) {
            LOG_INFO("SCAN", "Scan successful for label: %s", labels[currentLabel].c_str());
            break;
        }

        LOG_INFO("SCAN", "Accuracy insufficient. Retrying..");
        retryCount++;
    }

    if (!validScan) {
        LOG_ERROR("SCAN", "Scan failed after max retries : %d for label: %s", MAX_RETRIES_FOR_RSSI, labels[currentLabel].c_str());
    }

    return validScan;
}

// =======================================================
// MEASUREMENT COLLECTION ROUTER
// =======================================================

void collectMeasurements() {
    LOG_INFO("SCAN", "Collecting measurements based on current scanner mode...");

    switch (SystemSetup::currentSystemMode) {
        case STATIC_RSSI:
            LOG_INFO("SCAN", "Mode: Static RSSI");
            performStaticRSSIScan();
            break;

        case DYNAMIC_RSSI:
            LOG_INFO("SCAN", "Mode: Dynamic RSSI");
            performDynamicRSSIScan();
            break;

        case TOF:
            LOG_INFO("SCAN", "Mode: Time Of Flight");
            performTOFScan();
            break;

        default:
            LOG_INFO("SCAN", "Unsupported scanner mode: %d", SystemSetup::currentSystemScannerMode);
            break;
    }
}

// =======================================================
// SINGLE SCAN 
// =======================================================

void createSingleScan() {
    
    switch (SystemSetup::currentSystemScannerMode) {
        case STATIC_RSSI:
            LOG_INFO("SCAN", "Mode: Static RSSI");
            createSingleStaticRSSIScan();
            break;

        case DYNAMIC_RSSI:
            LOG_INFO("SCAN", "Mode: Dynamic RSSI");
            createSingleDynamicRSSIScan();
            break;

        case TOF:
            LOG_INFO("SCAN", "Mode: TOF");
            createSingleTOFScan();
            break;

        default:
            LOG_ERROR("SCAN", "Unsupported scanner mode: %d", SystemSetup::currentSystemScannerMode);
            break;
    }
}

// =======================================================
// RESCAN AFTER FAILED VALIDATION
// =======================================================

void rescan() {
    LOG_INFO("SCAN", "Rescanning after failed invalidation for label: %s", labels[currentLabel].c_str());

    DeleteBufferedData::scanner = SystemSetup::currentSystemScannerMode;
    //WARD: how to know how much to delete??  
      
    collectMeasurements();

    LOG_INFO("SCAN", "Rescan complete for label: %s", labels[currentLabel].c_str());
}
