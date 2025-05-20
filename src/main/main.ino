
#include <Arduino.h>
#include <vector>
#include <WiFi.h>
#include "../localisationMng/scanning.h"
#include "../localisationMng/predictionPhase.h"
#include "../sdCardBackup.h"

std::vector<Data> dataSet;
const char* anchorSSIDs[TOTAL_APS] = {
        "234/236",
        "236",
        "231/236",
        "231", 
        "kitchen",
        "entrance",
        "lobby",
        "balcony",
        "201",
        "offices"
};

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    if (!initSD()) {
        Serial.println("Fatal: SD init failed");
        while (true) delay(1000);
    }
    // 1) Check header format
    if (!verifyCSVFormat()) {
        Serial.printf(" %s header format invalid!\\n", SCAN_RESULTS_FILE);
    } else {
        Serial.printf(" %s header format valid\\n", SCAN_RESULTS_FILE);
    }

    // For each location, only scan if not already done:
    if (hasAllLocationRows()) {
        Serial.println("All locations present. Skipping scans.");
    } else {

        Serial.println("1 - Prediction Data Collection");

        for (int i = 0; i < NUMBER_OF_LOCATIONS; ++i) {
            LOCATIONS selectedLocation = promptLocationSelection();
            Serial.println("Press Enter to start scanning...");

            while (true) {
                if (Serial.available()) {
                    char c = Serial.read();
                    if (c == '\n' || c == '\r') break;
                }
            }

            Serial.println("Currently scanning location " + String((int) selectedLocation) + "\n");
            loadLocationDataset(selectedLocation);

            performScan(selectedLocation);
            saveLocationDataset(selectedLocation);
        }

        Serial.println("Data Collection Complete.");
        delay(1000);
    }
}

void loop() {
    Serial.println("Stand still to predict location. When you're ready, press Enter...");
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') break;
        }
    }

    int point[NUMBER_OF_ANCHORS] = {-100, -100, -100, -100, -100, -100, -100, -100, -100, -100}; 
    for (int sample = 0; sample < SAMPLES_PER_SCAN; ++sample) {
        int n = WiFi.scanNetworks();
        for (int j = 0; j < n; ++j) {
            String ssid = WiFi.SSID(j);
            int rssi = WiFi.RSSI(j);
            for (int k = 0; k < TOTAL_APS; ++k) {
                if (ssid.equals(anchorSSIDs[k])) {
                    point[k] = applyEMA(point[k], rssi);
                }
            }
        }
    }

    // Predict location and confirm with user
    preparePoint(point);
    LOCATIONS predictedLoc = static_cast<LOCATIONS>(knnPredict(point));
    confirmLocation(predictedLoc);
}
