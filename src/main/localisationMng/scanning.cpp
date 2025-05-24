#include "scanning.h"
#include "rssiScanner.h"
#include "tofScanner.h"
#include "userUI.h"

void runScanningPhase() {
    Serial.println();
    Serial.println("=============== Scanning Phase Started ===============");

    for (int i = 0; i < NUMBER_OF_LABELS; i++) {
        Label label = promptLocationLabel();
        currentScanningLabel = label;

        Serial.println();
        Serial.println("Selected Label: " + String(labelToString(label)));
        Serial.println(">> Press Enter to start scanning...");
        while (!Serial.available()) delay(50);
        Serial.read();  // consume newline

        Serial.println("Scanning Phase: Starting scan at " + String(labelToString(label)));
        startLabelScanningSession(label);
    }

    Serial.println();
    Serial.println("=============== Scanning Phase Completed ===============");
}

bool startLabelScanningSession(Label label) {
    int retryCount = 0;
    bool validScan = false;
    currentScanningLabel = label;

    while (retryCount < MAX_RETRIES) {
        Serial.println("Scanning Phase: performing scan attempt #" + String(retryCount + 1) +
                       " for: " + String(labelToString(label)));

        collectMeasurements();

        Serial.println("Scanning Phase: validating scan accuracy...");
        validScan = validateScanAccuracy();
        if (validScan) {
            Serial.println("Scanning Phase: scan completed successfully at: " + String(labelToString(label)));
            break;
        }

        Serial.println("Scanning Phase: accuracy insufficient, retrying scan...");
        retryCount++;
    }

    if (!validScan) {
        Serial.println("Scanning Phase: scan failed after max retries at: " + String(labelToString(label)));
        return false;
    }

    Serial.println("Scanning Phase: final accepted scan at " + String(labelToString(label)) +
                   " with accuracy: " + String(scanAccuracy) + "%");
    return true;
}

void collectMeasurements() {
    Serial.println("Scanning Phase: collecting measurements...");

    switch (Enablements::currentSystemState) {
        case STATIC_RSSI:
            Serial.println("Measurement Mode: STATIC_RSSI");
            performRSSIScan();
            break;

        case STATIC_RSSI_TOF:
            Serial.println("Measurement Mode: STATIC_RSSI_TOF");
            performRSSIScan();
            performTOFScan();
            break;

        case STATIC_DYNAMIC_RSSI:
            Serial.println("Measurement Mode: STATIC_DYNAMIC_RSSI");
            performRSSIScan();
            scanDynamicRSSI();
            break;

        case STATIC_DYNAMIC_RSSI_TOF:
            Serial.println("Measurement Mode: STATIC_DYNAMIC_RSSI_TOF");
            performRSSIScan();
            scanDynamicRSSI();
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
    Serial.println("Scanning Phase: Dynamic RSSI scanning is not implemented yet.");
}
