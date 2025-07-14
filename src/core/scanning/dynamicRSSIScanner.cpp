#include "dynamicRSSIScanner.h"
#include "core/prediction/predictionPhase.h"
#include "core/dataManaging/data.h"

#define EXPECTED_NUM_NEAR_APS 30

static bool notSoftAP(uint8_t* bssid) {
    // Examples of common SoftAP vendors (can be extended):
    return true;
    const uint8_t denylist[][3] = {
        {0x48, 0x4A, 0xE9}, 
        {0xD0, 0x15, 0xA6}
    };

    for (int i = 0; i < sizeof(denylist)/3; ++i) {
        if (bssid[0] == denylist[i][0] &&
            bssid[1] == denylist[i][1] &&
            bssid[2] == denylist[i][2]) {
            return true;
        }
    }
    return false;
}

void performDynamicRSSIScan() {
    resetDynamicRssiBuffer();
    for (int scan = 0; scan < DYNAMIC_RSSI_SCAN_BATCH_SIZE; scan++) {
        std::pair<DynamicRSSIData , DynamicMacData> scanData = createSingleDynamicRSSIScan();
        if(shouldAbort) {
            return;
        }
        SaveBufferedData::scanner = DYNAMIC_RSSI;
        SaveBufferedData::lastN++;
        saveData(scanData.second, scanData.first);

        LOG_DEBUG("Dynamic RSSI", "Scan %d for label %s", scan + 1, labels[currentLabel].c_str());
    }
    
    doneCollectingData();
}

std::pair<DynamicRSSIData , DynamicMacData> createSingleDynamicRSSIScan() {

     /* ---------- 0. Sanity-check Wi-Fi state ---------- */

    LOG_INFO("SCAN", "Start new scan");

    wifi_mode_t mode;
    if (esp_wifi_get_mode(&mode) != ESP_OK || (mode != WIFI_MODE_STA && mode != WIFI_MODE_APSTA)) {
        LOG_ERROR("SCAN", "Wi-Fi not in STA/AP-STA mode");
        shouldAbort = true;
        return {}; // empty pair, caller decides what to do
    }

    std::map<std::string, std::vector<int>> macToRssiMap;
    std::map<std::string, std::string> macToSsidMap;

    /* ---------- 2. Batch-scan loop ---------- */
    for (int sample = 0; sample < DYNAMIC_RSSI_SCAN_SAMPLE_PER_BATCH; ++sample) {
        const wifi_scan_config_t cfg = {
            .ssid        = nullptr,
            .bssid       = nullptr,
            .channel     = 0,
            .show_hidden = true
        };

        /* 2 a. Start scan (blocking) */
        esp_err_t err = esp_wifi_scan_start(&cfg, true);
        if (err != ESP_OK) {
            LOG_ERROR("SCAN", "scan_start failed (%s); skipping sample", esp_err_to_name(err));
            continue;                                   // don’t give up the whole batch
        }

        /* 2 b. Fetch results */
        wifi_ap_record_t ap_records[EXPECTED_NUM_NEAR_APS];
        uint16_t apCount = EXPECTED_NUM_NEAR_APS;
        err = esp_wifi_scan_get_ap_records(&apCount, ap_records);
        if (err != ESP_OK) {
            LOG_ERROR("SCAN", "get_ap_records failed (%s); skipping sample", esp_err_to_name(err));
            continue;
        }

        /* 2 c. Convert to (MAC,RSSI) pairs and keep strongest N */
        std::vector<std::pair<std::string,int>> macRssiPairs;
        macRssiPairs.reserve(apCount);

        for (uint16_t i = 0; i < apCount; ++i) {
            if(notSoftAP(ap_records[i].bssid)) {
                char macStr[18];
                sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
                    ap_records[i].bssid[0], ap_records[i].bssid[1], ap_records[i].bssid[2],
                    ap_records[i].bssid[3], ap_records[i].bssid[4], ap_records[i].bssid[5]);
                std::string ssidStr(reinterpret_cast<const char*>(ap_records[i].ssid), 33);
                macRssiPairs.emplace_back(std::string(macStr), ap_records[i].rssi);
                macToSsidMap[std::string(macStr)] = ssidStr;
            }
        }

        std::partial_sort(macRssiPairs.begin(),
                          macRssiPairs.begin() + std::min(NUMBER_OF_DYNAMIC_APS, (int)macRssiPairs.size()),
                          macRssiPairs.end(),
                          [](auto &a, auto &b){ return a.second > b.second; });

        for (int i = 0; i < std::min(NUMBER_OF_DYNAMIC_APS,(int)macRssiPairs.size()); ++i) {
            macToRssiMap[macRssiPairs[i].first].push_back(macRssiPairs[i].second);
        }
        
        vTaskDelay(pdMS_TO_TICKS(RSSI_SCAN_DELAY_MS));   // cooperative delay
    }

    /* ---------- 3. Post-process (EMA) ---------- */
    std::vector<std::pair<std::string,int>> processed;
    processed.reserve(macToRssiMap.size());

    for (auto &[mac, rssis] : macToRssiMap) {
        if (rssis.empty()) continue;
        int ema = rssis[0];
        for (size_t i = 1; i < rssis.size(); ++i)
            ema = applyEMA(ema, rssis[i]);
        processed.emplace_back(mac, ema);
    }

    std::sort(processed.begin(), processed.end(),
              [](auto &a, auto &b){ return a.second > b.second; });
    while (processed.size() > NUMBER_OF_DYNAMIC_APS) processed.pop_back();
    std::sort(processed.begin(), processed.end(),
          [](auto &a, auto &b){ return a.first < b.first; });

    for(auto &[mac, rssi] : processed) {
        LOG_DEBUG("SCAN" , " MAC address( %s ) | SSID( %s ) | RSSI( %d )  " , mac.c_str() , macToSsidMap[mac].c_str() , rssi);
    }
    
    /* ---------- 4. Build return structs ---------- */
    DynamicMacData   macData   = {};
    DynamicRSSIData  scanData  = {};
    macData.label = currentLabel;
    scanData.label = currentLabel;

    const int top = std::min(NUMBER_OF_DYNAMIC_APS, (int)processed.size());
    for (int i = 0; i < top; ++i) {
        const auto &macStr = processed[i].first;
        sscanf(macStr.c_str(), "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX",
               &macData.macAddresses[i][0], &macData.macAddresses[i][1], &macData.macAddresses[i][2],
               &macData.macAddresses[i][3], &macData.macAddresses[i][4], &macData.macAddresses[i][5]);

        scanData.RSSIs[i]             = processed[i].second;
        accumulatedDynamicRSSIs[i]    = processed[i].second;
        memcpy(accumulatedMacAddresses[i], macData.macAddresses[i], MAC_ADDRESS_SIZE);
    }

    for (int i = top; i < NUMBER_OF_DYNAMIC_APS; ++i) {
        memset(macData.macAddresses[i], 0, MAC_ADDRESS_SIZE);
        scanData.RSSIs[i]          = RSSI_DEFAULT_VALUE;
        accumulatedDynamicRSSIs[i] = RSSI_DEFAULT_VALUE;
        memset(accumulatedMacAddresses[i], 0, MAC_ADDRESS_SIZE);
    }

    return {scanData, macData};
}