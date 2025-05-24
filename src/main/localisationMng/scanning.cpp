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
    currentLabel = label;

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
    // Save the scan data to SD card

    AccuracyData record;
    record.location  = label;
    record.accuracy  = scanAccuracy;

   accuracyDatas.push_back(record);
    if (!saveAccuracyData(record)) {
        Serial.println("Failed to save accuracy data.");
    }

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

void scanStaticRSSI() {
    performRSSIScan();
}

void scanTOF() {
    performTOFScan();
}

void scanDynamicRSSI() {
    Serial.println("Dynamic RSSI scanning is not implemented yet.");
}