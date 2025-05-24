
#include <Arduino.h>
#include <vector>
#include <WiFi.h>
#include "../localisationMng/scanning.h"
#include "../localisationMng/predictionPhase.h"






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
  bool sdAvailable = initSDCard();       
///////////////////// new setup for sd ////////////////////
    if (sdAvailable){
         CurrDir = getSDBaseDir();

      if(SD.exists((CurrDir))) {
        loadLocationDataset();
        shouldReuseBackup=resetReuseFromSDForLabel();

        if(shouldReuseBackup){
            if(updateCSV()){
         Serial.println("Start using Backup data set! ");
         return;
        }
            
        }
     Serial.println("Backup data not relevant, Need to rescan all over");
         if(resetStorage()){
        Serial.println("Success : Creating new backup data for your new scans! ");
        clear.rssiDataSet
        clear.tofDataSet      
         }
         else{
        Serial.println("Failed to creater new Backup data ,");

         }
  }

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









----------------- new main --------------------


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

//1 global var should reuder
//2 map should be reused
//3 csv file and data set should 
//4)
        