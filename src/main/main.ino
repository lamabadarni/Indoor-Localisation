
#include <Arduino.h>
#include <vector>
#include <WiFi.h>
#include "../localisationMng/scanning.h"
#include "../localisationMng/predictionPhase.h"


// Globals 

String sessionFile;  // defined as extern in utilities.h
extern ScanConfig currentConfig;
std::vector<Data> dataSet;

String DataFile;
String metaFile;
String LocationAccuracyFile;

// Constants
SystemState currentSystemState = SystemState::STATIC_RSSI_TOF;

ScanConfig   currentConfig = {
    .IdRound        = "round_1",
    .systemState    = currentSystemState,
    .Count_Round     = 1,
    .RoundTimestamp  = millis(),
    .RSSINum       = ,
    .TOFNum        = 0

};


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

  // 1) Init SD (interactive if enabled)
  bool sdAvailable = initSDCard();            // sets sdAvailable
 // if (sdAvailable) pruneOldCSVs(FILES_THRESHOLD);

  // 2) Build per‐config filenames
    bool reuse = false;
    if (sdAvailable){
      if(SD.exists(systemStateToString(currentSystemState))) {
         // the purpose of reuse just to check if the meta data is relevant to curr config
        loadAccuracy();
        loadLocationDataset();
        isBackupDataSetRelevant()
        cleanUpCSVFile();
        cleanUpDataSet();

        createMetaFile();
        createCSVFile();

        Serial.println("No Backup data set relevant to current config");
      
    
  }
  // 3) If no SD, do fully interactive scan and exit setup

    for (int i = 0; i < NUMBER_OF_LOCATIONS; ++i) {
      LOCATIONS loc = promptLocationSelection();
      Serial.println("Press Enter to start scanning...");
      while (!Serial.available()) delay(50);
      Serial.read();  // clear newline
      Serial.printf(" Scanning location %d\n", (int)loc);

      performScan(sessionFile,loc, sdAvailable);
      Serial.printf(" Completed %u scans for loc %d\n", LocationScanBuffer.size(), (int)loc);
    }
    Serial.printf("INTERACTIVE SCAN COMPLETE: total=%u\n", dataSet.size());
    return;
  
  }

}

void confirmLocation(LOCATIONS loc) {
    Serial.printf("Predicted location: %s\n", locationToString(loc));
    Serial.println("Is this correct? (Y/N)");
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == 'Y' || c == 'y') {
                Serial.println("Location confirmed.");
                break;
            } else if (c == 'N' || c == 'n') {
                Serial.println("Location not confirmed.");
                break;
            }
        }
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









// ----------------- new main --------------------


void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("Indoor Localization System Starting...");
    
    // Step 1: Mode & Feature Setup
    SystemMode mode = promptSystemMode();
    setupEnablementsFromUser();

    // Step 2: Run mode-specific logic
    switch (mode) {
        case MODE_RSSI_MODEL_DIAGNOSTIC:
            verifyRSSIScanCoverage();
            break;

        case MODE_TOF_DIAGNOSTIC:
            verifyTOFScanCoverage();
            break;

        case MODE_TRAINING_ONLY:
        case MODE_FULL_SESSION:
            runTrainingSessionLoop();
            break;

        case MODE_PREDICTION_ONLY:
            runLivePredictionMode();
            break;

    }

}

        