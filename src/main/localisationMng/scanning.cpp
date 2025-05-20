#include "scanning.h"

const char* locationToString(int number) {
    static const char* locationNames[] = {
        "NOT_ACCURATE",
        "NEAR_ROOM_232",
        "NEAR_ROOM_234",
        "BETWEEN_ROOMS_234_236",
        "ROOM_236",
        "ROOM_231",
        "BETWEEN_ROOMS_231_236",
        "NEAR_BATHROOM",
        "NEAR_KITCHEN",
        "KITCHEN",
        "MAIN_ENTRANCE",
        "NEAR_ROOM_230",
        "LOBBY",
        "ROOM_201",
        "PRINTER",
        "MAIN_EXIT",
        "BALCONY_ENTRANCE",
        "OFFICES_HALL"
    };

    if (loc < 0 || loc >= sizeof(locationNames) / sizeof(locationNames[0])) {
        return "UNKNOWN_LOCATION";
    }
    return locationNames[loc];
}

// Apply Exponential Moving Average for RSSI smoothing
int applyEMA(int prevRSSI, int newRSSI) {
    if (prevRSSI == -100) return newRSSI;
    return (int)(ALPHA * prevRSSI + (1 - ALPHA) * newRSSI);
}

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
        scanData.location = locationLabel;
        for (int j = 0; j < TOTAL_APS; ++j) {
            scanData.RSSIs[j] = accumulatedRSSIs[j];
        }

        preparePoint(scanData.RSSIs);
        dataSet.push_back(scanData);
    }


}

char* promptLocationSelection() {
    Serial.println("Select Location:");
    for (int i = 1; i <= NUMBER_OF_LOCATIONS; ++i) {
        Serial.print(i);
        Serial.print(" - ");
        Serial.println(i); 
    }

    int loc = -1;
    while (loc < 0 || loc >= NUMBER_OF_LOCATIONS) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c >= '0' && c <= '9') loc = c - '0';
            if (c == '1') {
                if(Serial.peek() == '0') {
                    Serial.read();
                    loc = 10;
                }

                if(Serial.peek() == '1') {
                    Serial.read();
                    loc = 11;
                }

                if(Serial.peek() == '2') {
                    Serial.read();
                    loc = 12;
                }

                if(Serial.peek() == '3') {
                    Serial.read();
                    loc = 13;
                }

                if(Serial.peek() == '4') {
                    Serial.read();
                    loc = 14;
                }

                if(Serial.peek() == '5') {
                    Serial.read();
                    loc = 15;
                }

                if(Serial.peek() == '6') {
                    Serial.read();
                    loc = 16;
                }

                if(Serial.peek() == '7') {
                    Serial.read();
                    loc = 17;
                }
            }
        }
    }
    return locationToString(loc);
}

