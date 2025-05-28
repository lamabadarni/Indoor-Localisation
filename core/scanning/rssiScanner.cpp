#include "rssiScanner.h"
#include "../utils/platform.h"
#include "../utils/utilities.h"
#include "../prediction/predictionPhase.h"
#include "../dataManaging/data.h"
#include "../ui/logger.h"
#include "esp_wifi.h"

static void resetRssiBuffer() {
    for(int i = 0; i < NUMBER_OF_ANCHORS; i++) {
        accumulatedRSSIs[i] = RSSI_DEFAULT_VALUE;
    }
}

void performRSSIScan() {
    //start scanning - make 15 scan, each scan contains 3 samples and calculate EMA
    resetRssiBuffer();
    for (int scan = 0; scan < RSSI_SCAN_BATCH_SIZE; scan++) {
        
        RSSIData scanData = createSingleRSSIScan();
        saveData(scanData);
        
        LOG_DEBUG("RSSI", "Scan %d for label %s", scan + 1, labels[currentLabel]);
        for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
            LOG_DEBUG("RSSI", "SSID[%s], RSSI[%d] = %d", anchorSSIDs[i], i, scanData.RSSIs[i]);
        }
    }
}

RSSIData createSingleRSSIScan() {
    //create single scan as an EMA of 3 scans
    for (int sample = 0; sample < RSSI_SCAN_SAMPLE_PER_BATCH; ++sample) {
        wifi_scan_config_t scan_config = {
            .ssid = NULL,
            .bssid = NULL,
            .channel = 0,
            .show_hidden = true
        };

        ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true)); // blocking scan

        wifi_ap_record_t ap_records[NUMBER_OF_ANCHORS];
        uint16_t ap_num = NUMBER_OF_ANCHORS;
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));

        for (int j = 0; j < ap_num; ++j) {
            for (int k = 0; k < NUMBER_OF_ANCHORS; ++k) {
                if (strcmp((const char*)ap_records[j].ssid, anchorSSIDs[k].c_str()) == 0) {
                    accumulatedRSSIs[k] = applyEMA(accumulatedRSSIs[k], ap_records[j].rssi);
                }
            }
        }
        delay_ms(RSSI_SCAN_DELAY_MS);
    }

    RSSIData scanData;
    scanData.label = currentLabel;
    for (int i = 0; i < NUMBER_OF_ANCHORS; i++) {
        scanData.RSSIs[i] = accumulatedRSSIs[i];
    }
}

int computeRSSIPredictionMatches() {

    if(rssiDataSet.empty()) return 0;

    int matches = 0;
    bool mismatch[LABELS_COUNT] = {false};
    for(int sampleToPredict = 0; sampleToPredict < VALIDATION_MAX_ATTEMPTS; sampleToPredict++) {
        RSSIData scanData     = createSingleRSSIScan();
        Label predictedLabel  = rssiPredict();
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
        if(mismatch[m]) {
            LOG_INFO("SSID", " - %s.", labels[m]);
        }
        return;
    }

    return matches;
}