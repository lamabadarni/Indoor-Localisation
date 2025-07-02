#include "dynamicRSSIScanner.h"
#include "core/prediction/predictionPhase.h"
#include "core/dataManaging/data.h"

#define EXPECTED_NUM_NEAR_APS 30

void performDynamicRSSIScan() {
    resetDynamicRssiBuffer();
    for (int scan = 0; scan < DYNAMIC_RSSI_SCAN_SAMPLE_PER_BATCH; scan++) {
        std::pair<DynamicRSSIData , DynamicMacData> scanData = createSingleDynamicRSSIScan();
        SaveBufferedData::scanner = DYNAMIC_RSSI;
        SaveBufferedData::lastN++;
        saveData(scanData.second, scanData.first);

        LOG_DEBUG("Dynamic RSSI", "Scan %d for label %s", scan + 1, labels[currentLabel]);
    }
    
    doneCollectingData();
}

std::pair<DynamicRSSIData , DynamicMacData> createSingleDynamicRSSIScan() {
    std::map<std::string, std::vector<int>> macToRssiMap;

    for (int sample = 0; sample < DYNAMIC_RSSI_SCAN_SAMPLE_PER_BATCH; ++sample) {
        wifi_scan_config_t scan_config = {
            .ssid = NULL,
            .bssid = NULL,
            .channel = 0,
            .show_hidden = true
        };

        ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

        wifi_ap_record_t ap_records[EXPECTED_NUM_NEAR_APS];
        uint16_t ap_num = EXPECTED_NUM_NEAR_APS;
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));

        std::vector<std::pair<std::string, int>> macRssiPairs;
        for (int i = 0; i < ap_num; ++i) {
            char macStr[18];
            sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
                    ap_records[i].bssid[0], ap_records[i].bssid[1], ap_records[i].bssid[2],
                    ap_records[i].bssid[3], ap_records[i].bssid[4], ap_records[i].bssid[5]);
            macRssiPairs.emplace_back(macStr, ap_records[i].rssi);
        }

        std::sort(macRssiPairs.begin(), macRssiPairs.end(), [](auto &a, auto &b) {
            return a.second > b.second;
        });

        for (int i = 0; i < std::min(NUMBER_OF_DYNAMIC_APS, (int)macRssiPairs.size()); ++i) {
            macToRssiMap[macRssiPairs[i].first].push_back(macRssiPairs[i].second);
        }

        delay_ms(RSSI_SCAN_DELAY_MS);
    }

    std::vector<std::pair<std::string, int>> processedEntries;
    for (auto &[mac, rssis] : macToRssiMap) {
        int ema = rssis[0];
        for (size_t i = 1; i < rssis.size(); ++i) {
            ema = applyEMA(ema, rssis[i]);
        }
        processedEntries.emplace_back(mac, ema);
    }

    std::sort(processedEntries.begin(), processedEntries.end(), [](auto &a, auto &b) {
        return a.second > b.second;
    });

    int entryCount = std::min(NUMBER_OF_DYNAMIC_APS, (int)processedEntries.size());
    std::vector<std::pair<std::string, int>> topMacs(processedEntries.begin(), processedEntries.begin() + entryCount);

    std::sort(topMacs.begin(), topMacs.end(), [](auto &a, auto &b) {
        return a.first < b.first;
    });

    DynamicMacData macData;
    DynamicRSSIData scanData;
    macData.label = currentLabel;
    scanData.label = currentLabel;

    int idx = 0;
    for (auto &[macStr, rssi] : topMacs) {
        sscanf(macStr.c_str(), "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX",
               &macData.macAddresses[idx][0], &macData.macAddresses[idx][1], &macData.macAddresses[idx][2],
               &macData.macAddresses[idx][3], &macData.macAddresses[idx][4], &macData.macAddresses[idx][5]);
        scanData.RSSIs[idx] = rssi;
        idx++;
    }

    for (; idx < NUMBER_OF_DYNAMIC_APS; ++idx) {
        memset(macData.macAddresses[idx], 0, MAC_ADDRESS_SIZE);
        scanData.RSSIs[idx] = RSSI_DEFAULT_VALUE;
    }

    return std::pair<DynamicRSSIData, DynamicMacData>{scanData, macData};
}