/**
 * @file staticRSSIScanner.cpp
 * @brief RSSI scanning logic using Wi-Fi APs and Exponential Moving Average (EMA).
 *
 * Implements batched RSSI collection across known anchor SSIDs.
 * Each scan consists of multiple EMA-sampled values to smooth noise.
 * Populates global `accumulatedRSSIs` and writes `RSSIData` to buffer.
 */

#include "staticRSSIScanner.h"
#include "core/prediction/predictionPhase.h"
#include "core/dataManaging/data.h"

#define EXPECTED_NUM_NEAR_APS 30

void performStaticRSSIScan() {
    //start scanning - make 15 scan, each scan contains 3 samples and calculate EMA
    resetStaticRssiBuffer();
    for (int scan = 0; scan < STATIC_RSSI_SCAN_SAMPLE_PER_BATCH; scan++) {
        
        StaticRSSIData scanData = createSingleStaticRSSIScan();
        SaveBufferedData::scanner = STATIC_RSSI;
        SaveBufferedData::lastN++;
        saveData(scanData);

        LOG_DEBUG("RSSI", "Scan %d for label %s", scan + 1, labels[currentLabel]);
        for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
            LOG_DEBUG("RSSI", "SSID[%s], RSSI[%d] = %d", anchorSSIDs[i], i, scanData.RSSIs[i]);
        }
    }

    doneCollectingData();
}

StaticRSSIData createSingleStaticRSSIScan() {
    //create single scan as an EMA of 3 scans
    for (int sample = 0; sample < STATIC_RSSI_SCAN_SAMPLE_PER_BATCH; ++sample) {
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
                    accumulatedStaticRSSIs[k] = applyEMA(accumulatedStaticRSSIs[k], ap_records[j].rssi);
                } else {
                    accumulatedStaticRSSIs[k] = RSSI_DEFAULT_VALUE;
                }
            }
        }

        delay_ms(RSSI_SCAN_DELAY_MS);
    }

    StaticRSSIData scanData;
    scanData.label = currentLabel;
    for (int i = 0; i < NUMBER_OF_ANCHORS; i++) {
        scanData.RSSIs[i] = accumulatedStaticRSSIs[i];
    }
    return scanData;
}