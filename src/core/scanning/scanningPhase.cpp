/**
 * @file scanningPhase.cpp
 * @brief Implements scanning flow control, validation retries, and mode-specific measurement collection.
 *
 * Encapsulates the complete scanning logic across all user-defined labels,
 * including reuse handling, backup validation, and automatic retries on failure.
 * 
 */

#include "scanningPhase.h"
#include "rssiScanner.h"
#include "tofScanner.h"
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


    for (int i = 0; i < LABELS_COUNT; ++i) {
        delay_ms(DELAY_BETWEEN_PHASES);
        promptUserLocationLabel();  // Sets currentLabel

        // Backup data path
        if (reuseFromMemory[currentLabel]) {
            LOG_INFO("SCAN", "Backup data available for label: %s", labels[currentLabel].c_str());
            char input = promptUserReuseDecision();

            if (input == 'V' || input == 'v') {
                LOG_INFO("SCAN", "Checking backup data accuracy...");
                startLabelValidationSession();
                if (validForPredection[currentLabel]) continue; // Reuse backup if valid
            }

        }

        // Fresh scan path
        LOG_INFO("SCAN", "Selected label: %s", labels[currentLabel].c_str());
        LOG_INFO("SCAN", "Press any key to start scanning...");
        readCharFromUser();

        if (!startLabelScanningSession()) {
            LOG_WARN("SCAN", "Skipping label: %s due to repeated validation failure.", labels[currentLabel].c_str());
            skippedLabels.push_back(currentLabel);
        }
    }

    LOG_INFO("SCAN", "=============== Scanning Phase Completed ===============");

    if (!skippedLabels.empty()) {
        LOG_WARN("SCAN", "The following labels were skipped due to scan failure:");
        for (Label skipped : skippedLabels) {
            LOG_WARN("SCAN", " - %s | Accuracy: %.2f%%", labels[skipped].c_str(), getAccuracy());
        }
    } else {
        LOG_INFO("SCAN", "All labels were scanned successfully.");
    }
}

// =======================================================
// SINGLE LABEL SESSION W/ RETRIES
// =======================================================

bool startLabelScanningSession() {
    int retryCount = 0;
    bool validScan = false;

    if (!dontAskAgain) {
        char input = promptUserRunCoverageDiagnostic();
        if (input == 'Y' || input == 'y') performRSSIScanCoverage();
        if (input == 'D' || input == 'd') dontAskAgain = true;
    }

    while (retryCount < MAX_RETRIES_FOR_RSSI) {
        LOG_INFO("SCAN", "Attempt #%d for label: %s", retryCount + 1, labels[currentLabel].c_str());

        collectMeasurements();

        LOG_INFO("SCAN", "Validating scan accuracy...");
        startLabelValidationSession();

        if (validScan) {
            LOG_INFO("SCAN", "Scan successful for label: %s", labels[currentLabel].c_str());
            break;
        }

        LOG_WARN("SCAN", "Accuracy insufficient. Retrying...");
        retryCount++;
    }

    if (!validScan) {
        LOG_ERROR("SCAN", "Scan failed after max retries for label: %s", labels[currentLabel].c_str());
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
            LOG_INFO("SCAN", "Mode: STATIC_RSSI");
            performRSSIScan();
            break;

        case TOF:
            LOG_INFO("SCAN", "Mode: TOF");
            performTOFScan();
            break;

        case STATIC_RSSI_TOF:
            LOG_INFO("SCAN", "Mode: STATIC_RSSI_TOF");
            performRSSIScan();
            performTOFScan();
            break;

        default:
            LOG_WARN("SCAN", "Unsupported scanner mode: %d", SystemSetup::currentSystemScannerMode);
            break;
    }
}

// =======================================================
// SINGLE QUICK SCAN (DIAGNOSTIC)
// =======================================================

void createSingleScan() {
    switch (SystemSetup::currentSystemScannerMode) {
        case STATIC_RSSI:
            LOG_INFO("SCAN", "Mode: STATIC_RSSI");
            createSingleRSSIScan();
            break;

        case TOF:
            LOG_INFO("SCAN", "Mode: TOF");
            createSingleTOFScan();
            break;

        case STATIC_RSSI_TOF:
            LOG_INFO("SCAN", "Mode: STATIC_RSSI_TOF");
            createSingleRSSIScan();
            createSingleTOFScan();
            break;

        default:
            LOG_WARN("SCAN", "Unsupported scanner mode: %d", SystemSetup::currentSystemScannerMode);
            break;
    }
}

// =======================================================
// RESCAN AFTER FAILED VALIDATION
// =======================================================

void rescan() {
    LOG_INFO("SCAN", "Rescanning after failed invalidation for label: %s", labels[currentLabel].c_str());

    // TODO: Consider clearing invalid measurements if needed

    collectMeasurements();

    LOG_INFO("SCAN", "Rescan complete for label: %s", labels[currentLabel].c_str());
}
