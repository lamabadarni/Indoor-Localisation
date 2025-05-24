/**
 * @file rssiScanner.cpp
 * @brief Performs RSSI data collection, EMA smoothing, prediction input formatting, and validation logic for anchors.
 * @author Lama Badarni
 */

#include "rssiScanner.h"

static void resetRssiBuffer(int rssi[NUMBER_OF_ANCHORS]) {
    for(int i = 0; i < NUMBER_OF_ANCHORS; i++) {
        rssi[i] = RSSI_DEFAULT_VALUE;
    }
}

void performRSSIScan() {
    //start scanning - make 15 scan, each scan contains 3 samples and calculate EMA
    for (int scan = 0; scan < SCAN_BATCH_SIZE; ++scan) {
        resetRssiBuffer(accumulatedRSSIs);
        for(int sample = 0; sample < SAMPLE_PER_SCAN_BATCH; ++sample) {
            int n = WiFi.scanNetworks();
            for (int j = 0; j < n; ++j) {
                String ssid = WiFi.SSID(j);
                int rssi = WiFi.RSSI(j);
                for (int k = 0; k < NUMBER_OF_ANCHORS; ++k) {
                    if (ssid.equals(anchorSSIDs[k])) {
                        accumulatedRSSIs[k] = applyEMA(accumulatedRSSIs[k], rssi);
                    }
                }
            }
            WiFi.scanDelete();
            delay(SCAN_DELAY_MS);
        }

        //Add each scan to data set
        RSSIData scanData;
        scanData.label = currentLabel;
        for (int j = 0; j < NUMBER_OF_ANCHORS; ++j) {
            scanData.RSSIs[j] = accumulatedRSSIs[j];
        }
        rssiDataSet.push_back(scanData);
        saveRSSIScan(scanData);

        Serial.println("[RSSI] Scan " + String(scan + 1) + " for label " + String(labelToString(currentLabel)) + ":");
        for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
            Serial.print(String(scanData.RSSIs[i]) + "  ");
        }
        Serial.println();
    }
}

int computeRSSIPredictionMatches() {
    int matches = 0;

    for(int sampleToPredict = 0; sampleToPredict < NUM_OF_VALIDATION_SCANS; sampleToPredict++) {
        createRSSIScanToMakePrediction();
        Label predictedLabel = rssiPredict();
        if(predictedLabel == currentLabel) {
            matches++;

            //Add each scan to scan data if the predection succeeded
            RSSIData scanData;
            scanData.label = currentLabel;
            for (int j = 0; j < NUMBER_OF_ANCHORS; ++j) {
                scanData.RSSIs[j] = accumulatedRSSIs[j];
            }

            rssiDataSet.push_back(scanData);
            saveRSSIScan(scanData);
        }

        Serial.println("[RSSI VALIDATION] #" + String(sampleToPredict + 1) +
               ": Predicted " + String(labelToString(predictedLabel)) +
               " | Actual " + String(labelToString(currentLabel)));
    }
    return matches;
}

void createRSSIScanToMakePrediction() {
    resetRssiBuffer(accumulatedRSSIs);
    for(int sample = 0; sample < RSSI_SCAN_SAMPLE_PER_BATCH; ++sample) {
        int n = WiFi.scanNetworks();
        for (int j = 0; j < n; ++j) {
            String ssid = WiFi.SSID(j);
            int rssi = WiFi.RSSI(j);
            for (int k = 0; k < NUMBER_OF_ANCHORS; ++k) {
                if (ssid.equals(anchorSSIDs[k])) {
                    accumulatedRSSIs[k] = applyEMA(accumulatedRSSIs[k], rssi);
                }
            }
        }
        WiFi.scanDelete();
        delay(RSSI_SCAN_DELAY_MS);
    }
}