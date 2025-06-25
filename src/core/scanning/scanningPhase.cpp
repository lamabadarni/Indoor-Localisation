#include "scanningPhase.h"
#include "rssiScanner.h"
#include "tofScanner.h"
#include "../ui/userUI.h"
#include "../validation/validationPhase.h"
#include "../utils/utilities.h"
#include "../utils/logger.h"

static bool dontAskAgain = false;

void runScanningPhase() {
    delay_ms(DELAY_BETWEEN_PHASES);

    LOG_INFO("SCAN", "=============== Scanning Phase Started ===============");
    LOG_INFO("SCAN", "Please stand still at the selected location during scanning.");
    LOG_INFO("SCAN", "Collected data will be used for training.");

    for (int i = 0; i < LABELS_COUNT; ++i) {
        delay_ms(DELAY_BETWEEN_PHASES);

        promptUserLocationLabel();  // Sets currentLabel
        if (reuseFromMemory[currentLabel]) {
            LOG_INFO("SCAN", "Backup data available for label: %s", labels[currentLabel].c_str());
            char input = promptUserReuseDecision();
            if (input == 'N' || input == 'n') continue;
            if (input == 'V' || input == 'v') {
                bool accurate = validateScanAccuracy();
                if(accurate) {
                    
                }
                continue;
            }
        }

        LOG_INFO("SCAN", "Selected Label: %s", labels[currentLabel].c_str());
        LOG_INFO("SCAN", "Press any key to start scanning...");
        readCharFromUser();

        if (!startLabelScanningSession()) {
            LOG_WARN("SCAN", "Skipping label due to repeated validation failure.");
        }
    }

    LOG_INFO("SCAN", "=============== Scanning Phase Completed ===============");
}

bool startLabelScanningSession() {
    int retryCount = 0;
    bool validScan = false;

    if (!dontAskAgain) {
        char input = promptUserRunCoverageDiagnostic();
        if (input == 'Y' || input == 'y') performRSSIScan();
        if (input == 'D' || input == 'd') dontAskAgain = true;
    }

    while (retryCount < MAX_RETRIES_FOR_RSSI) {
        LOG_INFO("SCAN", "Attempt #%d for label: %s", retryCount + 1, labels[currentLabel].c_str());

        collectMeasurements();

        LOG_INFO("SCAN", "Validating scan accuracy...");
        startLabelValidationSession();

        bool validScan = (getAccuracy() > VALIDATION_PASS_THRESHOLD);

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

void collectMeasurements() {
    LOG_INFO("SCAN", "Collecting measurements based on current state...");

    switch (SystemSetup::currentSystemMode) {
        case STATIC_RSSI:
            LOG_INFO("SCAN", "System Mode: STATIC_RSSI");
            performRSSIScan();
            break;

        case TOF:
            LOG_INFO("SCAN", "System Mode: TOF");
            performTOFScan();
            break;

        case STATIC_RSSI_TOF:
            LOG_INFO("SCAN", "System Mode: STATIC_RSSI_TOF");
            performRSSIScan();
            performTOFScan();
            break;

        default:
            LOG_WARN("SCAN", "Unsupported system state: %d", SystemSetup::currentSystemScannerMode);
            break;
    }
}

void createSingleScan() {
    switch (SystemSetup::currentSystemScannerMode) {
        case STATIC_RSSI:
            LOG_INFO("SCAN", "System Mode: STATIC_RSSI");
            createSingleRSSIScan();
            break;

        case TOF:
            LOG_INFO("SCAN", "System Mode: TOF");
            createSingleTOFScan();
            break;

        case STATIC_RSSI_TOF:
            LOG_INFO("SCAN", "System Mode: STATIC_RSSI_TOF");
            createSingleRSSIScan();
            createSingleTOFScan();
            break;

        default:
            LOG_WARN("SCAN", "Unsupported system state: %d", SystemSetup::currentSystemScannerMode);
            break;
    }
}

void rescan() {

    LOG_INFO("SCAN", "Rescanning after failed invalidation for label: %s", labels[currentLabel].c_str());

    //ward :: here , should we delete the meas that failed?

    collectMeasurements();

    LOG_INFO("SCAN", "Done rescanning after failed invalidation for label: %s", labels[currentLabel].c_str());

    return;
}

/*
double computeScanAccuracy() {
    int matchesRSSI = -1;
    int matchesTOF = -1;
    double accuracy = 0;

    switch (SystemSetup::currentSystemScannerMode) {
        case STATIC_RSSI:
            matchesRSSI = computeRSSIPredictionMatches();
            accuracy    = matchesRSSI / VALIDATION_MAX_ATTEMPTS;
            break;
        case TOF :
            matchesTOF = computeTOFPredictionMatches();
            accuracy   = matchesTOF / VALIDATION_MAX_ATTEMPTS;
        case STATIC_RSSI_TOF:
            matchesRSSI = computeRSSIPredictionMatches();
            matchesTOF  = computeTOFPredictionMatches();
            accuracy    = (matchesRSSI + matchesTOF) / VALIDATION_MAX_ATTEMPTS;
            break;
    }
}
*/