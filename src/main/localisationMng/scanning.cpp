#include "scanning.h"
#include "rssiScanner.h"
#include "tofScanner.h"
#include "userUI.h"

static int scanAccuracy = 0;

/**
 * @brief Starts a scanning session with retries and validation.
 *        Collects measurements and checks prediction accuracy.
 */
bool startLabelScanningSession(Label label) {
    int retryCount = 0;
    bool validScan = false;
    currentScanningLabel = label;

    while (retryCount < MAX_RETRIES) {
        Serial.printf("Scanning Phase: performing scan attempt #%d for: %s\n", retryCount + 1, labelToString(label));

        collectMeasurements();

        Serial.println("Scanning Phase: validating scan accuracy...");
        validScan = validateScanAccuracy();
        if (validScan) {
            Serial.printf("Scanning Phase: scan completed successfully at: %s\n", labelToString(label));
            break;
        }

        Serial.println("Scanning Phase: accuracy insufficient, retrying scan...");
        retryCount++;
    }

    if (!validScan) {
        Serial.println("Scanning Phase: scan failed after max retries.");
        return false;
    }

    Serial.printf("Scanning Phase: final accepted scan at %s with accuracy: %d%%\n", labelToString(label), scanAccuracy);
    return true;
}

/**
 * @brief Collects measurements based on current system state.
 *        Dispatches to appropriate scanner modules.
 */
void collectMeasurements() {
    Serial.println("Scanning Phase: collecting measurements...");

    switch (Enablements::currentSystemState) {
        case STATIC_RSSI:
            performRSSIScan();
            break;

        case STATIC_RSSI_TOF:
            performRSSIScan();
            performTOFScan();
            break;

        case STATIC_DYNAMIC_RSSI:
            performRSSIScan();
            scanDynamicRSSI();  // Placeholder for future feature
            break;

        case STATIC_DYNAMIC_RSSI_TOF:
            performRSSIScan();
            scanDynamicRSSI();  // Placeholder for future feature
            performTOFScan();
            break;

        default:
            Serial.println("Scanning Phase: Unsupported system state.");
            break;
    }
}

/**
 * @brief Validates the scan results by checking how many predictions match the label.
 *        Uses both RSSI and TOF modules if enabled. If failed, offers fallback.
 */
bool validateScanAccuracy() {
    int matchesRSSI = 0;
    int matchesTOF = 0;
    bool combinedOK = false;

    // Run RSSI and/or TOF predictions depending on system state
    switch (Enablements::currentSystemState) {
        case STATIC_RSSI:
            matchesRSSI = computeRSSIPredictionMatches();
            break;

        case STATIC_RSSI_TOF:
            matchesRSSI = computeRSSIPredictionMatches();
            matchesTOF = computeTOFPredictionMatches();
            break;

        case STATIC_DYNAMIC_RSSI:
            matchesRSSI = computeRSSIPredictionMatches();
            break;

        case STATIC_DYNAMIC_RSSI_TOF:
            matchesRSSI = computeRSSIPredictionMatches();
            matchesTOF = computeTOFPredictionMatches();
            break;

        default:
            break;
    }

    int totalMatches = matchesRSSI + matchesTOF;
    int totalAttempts = (matchesTOF > 0) ? 2 * SCAN_VALIDATION_SAMPLE_SIZE : SCAN_VALIDATION_SAMPLE_SIZE;
    scanAccuracy = (100 * totalMatches) / totalAttempts;

    Serial.printf("Scanning Phase: combined accuracy = %d%% (%d/%d correct predictions)\n",
                  scanAccuracy, totalMatches, totalAttempts);

    combinedOK = (totalMatches >= VALIDATION_PASS_THRESHOLD);

    if (combinedOK && promptUserAccuracyApprove()) return true;

    // Combined validation failed — fallback prompt
    int choice = promptRetryValidationWithSingleMethod();

    if (choice == 1) {
        Serial.println("Retrying validation with RSSI only...");
        matchesRSSI = computeRSSIPredictionMatches();
        scanAccuracy = (100 * matchesRSSI) / SCAN_VALIDATION_SAMPLE_SIZE;
        Serial.printf("[RSSI] Retry accuracy = %d%%\n", scanAccuracy);
        return (matchesRSSI >= VALIDATION_PASS_THRESHOLD) && promptUserAccuracyApprove();
    }

    if (choice == 2) {
        Serial.println("Retrying validation with TOF only...");
        matchesTOF = computeTOFPredictionMatches();
        scanAccuracy = (100 * matchesTOF) / SCAN_VALIDATION_SAMPLE_SIZE;
        Serial.printf("[TOF] Retry accuracy = %d%%\n", scanAccuracy);
        return (matchesTOF >= VALIDATION_PASS_THRESHOLD) && promptUserAccuracyApprove();
    }

    Serial.println("User aborted validation.");
    return false;
}

/**
 * @brief Placeholder for dynamic RSSI scanning logic (future feature).
 */
void scanDynamicRSSI() {
    Serial.println("Dynamic RSSI scanning is not implemented yet.");
}

/**
 * @brief Calls TOF scanning module (for logical symmetry).
 */
void scanTOF() {
    performTOFScan();
}
