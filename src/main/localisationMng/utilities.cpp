/*
*  @author: Lama
 * @file utillities.cpp
 * @brief static functions implementation
 */

 #include "utilities.cpp"

// Map specefic label to string 
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

 
// User UI for choosing label
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

SystemState promptSystemState() {
    Serial.println("Select System State:");
    Serial.println("0 - STATIC_RSSI");
    Serial.println("1 - STATIC_RSSI_TOF");
    Serial.println("2 - STATIC_DYNAMIC_RSSI");
    Serial.println("3 - STATIC_DYNAMIC_RSSI_TOF");
    Serial.println("4 - OFFLINE");

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c >= '0' && c <= '4') return (SystemState)(c - '0');
        }
    }
}

const char* systemStateToString(int state) {
    switch (state) {
        case STATIC_RSSI: return "STATIC_RSSI";
        case STATIC_RSSI_TOF: return "STATIC_RSSI_TOF";
        case STATIC_DYNAMIC_RSSI: return "STATIC_DYNAMIC_RSSI";
        case STATIC_DYNAMIC_RSSI_TOF: return "STATIC_DYNAMIC_RSSI_TOF";
        case OFFLINE: return "OFFLINE";
        default: return "UNKNOWN";
    }
}

// Apply Exponential Moving Average for RSSI smoothing
int applyEMA(int prevRSSI, int newRSSI) {
    if (prevRSSI == -100) return newRSSI;
    return (int)(ALPHA * prevRSSI + (1 - ALPHA) * newRSSI);
}

