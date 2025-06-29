/**
 * @file rssiScanner.cpp
 * @brief RSSI scanning logic using Wi-Fi APs and Exponential Moving Average (EMA).
 *
 * Implements batched RSSI collection across known anchor SSIDs.
 * Each scan consists of multiple EMA-sampled values to smooth noise.
 * Populates global `accumulatedRSSIs` and writes `RSSIData` to buffer.
 */

#include "rssiScanner.h"
#include "core/prediction/predictionPhase.h"
#include "core/dataManaging/data.h"

#define EXPECTED_NUM_NEAR_APS 30

void performRSSIScan() {
    //start scanning - make 15 scan, each scan contains 3 samples and calculate EMA
    resetRssiBuffer();
    for (int scan = 0; scan < RSSI_SCAN_BATCH_SIZE; scan++) {
        
        RSSIData scanData = createSingleRSSIScan();
        BufferedData::scanner = STATICRSSI;
        BufferedData::lastN++;
        saveData(scanData);

        LOG_DEBUG("RSSI", "Scan %d for label %s", scan + 1, labels[currentLabel]);
        for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
            LOG_DEBUG("RSSI", "SSID[%s], RSSI[%d] = %d", anchorSSIDs[i], i, scanData.RSSIs[i]);
        }
    }

    doneCollectingData();
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

        wifi_ap_record_t ap_records[EXPECTED_NUM_NEAR_APS];
        u_int16_t ap_num = EXPECTED_NUM_NEAR_APS;

        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));

        for (int j = 0; j < ap_num; ++j) {
            for (int k = 0; k < NUMBER_OF_ANCHORS; ++k) {
                if (sample > 0 && strcmp((const char*)ap_records[j].ssid, anchorSSIDs[k].c_str()) == 0) {
                    accumulatedRSSIs[k] = applyEMA(accumulatedRSSIs[k], ap_records[j].rssi);
                } else {
                    accumulatedRSSIs[k] = RSSI_DEFAULT_VALUE;
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

    return scanData;
}