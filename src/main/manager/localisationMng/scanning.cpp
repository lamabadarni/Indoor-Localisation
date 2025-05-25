/**
 * @file scanning.cpp
 * @brief Manages full scanning sessions across labels, handles retries, validation, and measurement dispatching.
 * @author Lama Badarni
 */

#include <Accuracy_data_manger.h>
#include <scanning.h>
#include <rssiScanner.h>
#include <userUI.h>
#include <validationPhase.h>

void runScanningPhase() {
    Serial.println();
    Serial.println("=============== Scanning Phase Started ===============");

    for (int i = 0; i < NUMBER_OF_LABELS; i++) {
        currentLabel = promptLocationLabel();
        if(reuseFromSD[currentLabel]) {
            continue;
        }
        Serial.println();
        Serial.println("[SCAN] Selected Label: " + String(labelToString(currentLabel)));
        Serial.println("[SCAN] >> Press Enter to start scanning...");
        while (!Serial.available()) delay(50);
        Serial.read();

        Serial.println("[SCAN] Starting scan at: " + String(labelToString(currentLabel)));
        startLabelScanningSession();
    }

    Serial.println();
    Serial.println("=============== Scanning Phase Completed ===============");
}

bool startLabelScanningSession() {
    int retryCount = 0;
    bool validScan = false;

    while (retryCount < MAX_RETRIES) {
        Serial.println("[SCAN] Attempt #" + String(retryCount + 1) + " for: " + String(labelToString(currentLabel)));

        collectMeasurements();

        Serial.println("[SCAN] Validating scan accuracy...");
        validScan = validateScanAccuracy();
        if (validScan) {
            Serial.println("[SCAN] Scan successful at: " + String(labelToString(currentLabel)));
            break;
        }

        Serial.println("[SCAN] Accuracy insufficient, retrying...");
        retryCount++;
    }

    if (!validScan) {
        Serial.println("[SCAN] Scan failed after max retries at: " + String(labelToString(currentLabel)));
        return false;
    }

    Serial.println("[SCAN] Final accepted scan at: " + String(labelToString(currentLabel)) + ", Accuracy: " + String(accuracy) + "%");
    /*
    AccuracyData record;
    record.location = currentLabel;
    record.accuracy = accuracy;

    accuracyDatas.push_back(record);
    if (!saveAccuracyData(record)) {
        Serial.println("[SCAN] Failed to save accuracy data.");
    }
        */
    return true;
}

void collectMeasurements() {
    Serial.println("[SCAN] Collecting measurements...");

    switch (currentSystemState) {
        case STATIC_RSSI:
            Serial.println("[SCAN] Mode: STATIC_RSSI");
            performRSSIScan();
            break;

        case STATIC_RSSI_TOF:
            Serial.println("[SCAN] Mode: STATIC_RSSI_TOF");
            performRSSIScan();
            //performTOFScan();
            break;

        case STATIC_DYNAMIC_RSSI:
            Serial.println("[SCAN] Mode: STATIC_DYNAMIC_RSSI");
            performRSSIScan();
            scanDynamicRSSI();
            break;

        case STATIC_DYNAMIC_RSSI_TOF:
            Serial.println("[SCAN] Mode: STATIC_DYNAMIC_RSSI_TOF");
            performRSSIScan();
            scanDynamicRSSI();
            //performTOFScan();
            break;

        default:
            Serial.println("[SCAN] Unsupported system state.");
            break;
    }
}

void scanStaticRSSI() {
    performRSSIScan();
}

void scanTOF() {
    Serial.println("[SCAN] FTM not implemented.");
}

void scanDynamicRSSI() {
   Serial.println("[SCAN] Dynamic RSSI scanning not implemented.");
}
