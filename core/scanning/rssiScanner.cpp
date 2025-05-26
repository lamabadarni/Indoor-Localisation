/**
 * @file rssiScanner.cpp
 * @brief Performs RSSI data collection, EMA smoothing, prediction input formatting, and validation logic for anchors.
 * @author Lama Badarni
 */

#include "rssiScanner.h"
#include "../utils/platform.h"
#include "../utils/utilities.h"
#include "../prediction/predictionPhase.h"
#include "../dataManaging/data.h"
#include <../dataManaging/sdCardIO.h>

static void resetRssiBuffer() {
    for(int i = 0; i < NUMBER_OF_ANCHORS; i++) {
        accumulatedRSSIs[i] = RSSI_DEFAULT_VALUE;
    }
}

void performRSSIScan() {
    //start scanning - make 15 scan, each scan contains 3 samples and calculate EMA
    resetRssiBuffer();
    for (int scan = 0; scan < RSSI_SCAN_BATCH_SIZE; scan++) {
        
        RSSIData scanData = createRSSIScan();
        saveData(scanData);
        
        LOG_DEBUG("RSSI", "Scan %d for label %s", scan + 1, labelToString(currentLabel));
        for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
            LOG_DEBUG("RSSI", "SSID[%s], RSSI[%d] = %d", anchorSSIDs[i] i, scanData.RSSIs[i]);
        }
    }
}

RSSIData createSingleRSSIScan() {
    //create single scan as an EMA of 3 scans
    for(int sample = 0; sample < RSSI_SCAN_SAMPLE_PER_BATCH; ++sample) {
        int n = WiFi.scanNetworks();
        for (int j = 0; j < n; ++j) {
            string ssid = WiFi.SSID(j);
            int rssi = WiFi.RSSI(j);
            for (int k = 0; k < NUMBER_OF_ANCHORS; ++k) {
                if (ssid.equals(anchorSSIDs[k])) {
                    accumulatedRSSIs[k] = applyEMA(accumulatedRSSIs[k], rssi);
                }
            }
        }
        WiFi.scanDelete();
        delay_ms(RSSI_SCAN_DELAY_MS);
    }

    RSSIData scanData;
    scanData.label = currentLabel;
    for (int i = 0; i < NUMBER_OF_ANCHORS; i++) {
        scanData.RSSIs[j] = accumulatedRSSIs[j];
    }
}

int computeRSSIPredictionMatches() {
    int matches = 0;
    bool mismatch[LABELS_COUNT] = {false};
    for(int sampleToPredict = 0; sampleToPredict < VALIDATION_MAX_ATTEMPTS; sampleToPredict++) {
        RSSIData scanData    = createSingleRSSIScan();
        Label predictedLabel = rssiPredict();
        if(predictedLabel == currentLabel) {
            matches++;
            //Add each scan to scan data if the predection succeeded
            saveToBuffer();
        }
        else {
            mismatch[predictedLabel] = true;
        }

        LOG_DEBUG("RSSI", "[VALIDATION] #%d: Predicted %s | Actual %s",
            sampleToPredict + 1, labels[predictedLabel], labels[currentLabel]);
         
    } 

    if(matches == VALIDATION_MAX_ATTEMPTS) {
        LOG_INFO("RSSI", "All predictions matched expected %s.", labels[currentLabel]);
        return;
    }

    LOG_INFO("RSSI", "Mismatched labels:");
    for(int m = 0; m < LABELS_COUNT; m++) {
        LOG_INFO("RSSI", " - %s.", labels[m]);
        return;
    }

    return matches;
}

void printAcc