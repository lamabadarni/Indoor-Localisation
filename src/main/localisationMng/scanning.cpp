#include "scanning.h"

void performScan(LOCATIONS locationLabel) {
    for (int i = 0; i < SCANS_PER_LOCATION; ++i) {
        int accumulatedRSSIs[TOTAL_APS];
        Serial.println("starting iteration " + String(i) + " at location " + locationToString((int)locationLabel) "\n");
        for (int j = 0; j < TOTAL_APS; ++j) accumulatedRSSIs[j] = -100;

        for (int sample = 0; sample < SAMPLES_PER_SCAN; ++sample) {
            int n = WiFi.scanNetworks();

            for (int j = 0; j < n; ++j) {
                String ssid = WiFi.SSID(j);
                int rssi = WiFi.RSSI(j);

                for (int k = 0; k < TOTAL_APS; ++k) {
                    if (ssid.equals(anchorSSIDs[k])) {
                        accumulatedRSSIs[k] = applyEMA(accumulatedRSSIs[k], rssi);
                        Serial.println("Anchor" + anchorSSIDs[k] + " RSSI " + String(accumulatedRSSIs[k]) "\n");
                    }
                }
            }
            WiFi.scanDelete();
            delay(SCAN_DELAY_MS);
        }

        Data scanData;
        scanData.label = locationLabel;
        for (int j = 0; j < TOTAL_APS; ++j) {
            scanData.RSSIs[j] = accumulatedRSSIs[j];
        }

        preparePoint(scanData.RSSIs);
        dataSet.push_back(scanData);
    }
}


void performScan(LOCATIONS locationLabel) {
    int retryCount = 0;

    while(retryCount < MAX_RETRIES) {
        Serial.println("Scanning Phase: perfroming scan attempt #" + String(retryCount + 1));
        
    }
}
