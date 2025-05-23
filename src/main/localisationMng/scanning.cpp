#include "scanning.h"

static int scanAccuracy = 0;


static void resetRssiBuffer(int rssi[NUMBER_OF_ANCHORS]) {
    for(int i = 0; i < NUMBER_OF_ANCHORS; i++) {
        rssi[i] = DEFAULT_RSSI_VALUE;
    }
}

bool startLabelScanningSession(Label label) {
    int retryCount       = 0;
    bool validScan       = false;

    while(retryCount < MAX_RETRIES) {
        Serial.println("Scanning Phase: perfroming scan attempt #" + String(retryCount + 1) + " for: " locationToString(currentScanningLabel));
        collectMeasurements();
        Serial.println("Scanning Phase: scan attempt #" + String(retryCount + 1) + " checking scan validity");
        validScan = computePredictionMatches();
        if(validScan) {
            Serial.println("Scanning Phase: scan completed at: " + locationToString(currentScanningLabel));
            break;
        }
        Serial.println("Scanning Phase: previous scan accuracy were not sufficient, produce another scan");
    }

    //If we've reached max retries and scan still unvalid
    if(!validScan) {
        Serial.println("Scanning Phase: scan failed!");
        return false;
    }

    Serial.println("Scanning Phase: scan success :) Scan accuracy: " String(scanAccuracy) + "%");
    return true;
}

void collectMeasurements() {
    Serial.println("Scanning Phase: start scanning ...");
    switch(Enablements::currentSystemState) {
        case(SystemState::STATIC_RSSI) {
            scanStaticRSSI();
            break;
        }
        case(SystemState::STATIC_RSSI_TOF) {
            scanStaticRSSI();
            scanTOF();
            break;;
        }
        case(SystemState::STATIC_DYNAMIC_RSSI) {
            scanStaticRSSI();
            scanDynamicRSSI();
            break;
        }

        case(SystemState::STATIC_DYNAMIC_RSSI_TOF) {
            scanStaticRSSI();
            scanTOF();
            scanDynamicRSSI();
            break;
        }
    } 
}

int computeRSSIPredictionMatches() {
    int predectionSuccess = 0;

    for(int sampleToPredict = 0; sampleToPredict < SCAN_VALIDATION_SAMPLE_SIZE; sampleToPredict++) {
        int rssiPointToPredict[NUMBER_OF_ANCHORS] = createRSSIScanToMakePredection();
        Label predictedLabel = rssiPredict(rssiPointToPredict);
        if(predictedLabel == currentScanningLabel) {
            predectionSuccess++;

            //Add each scan to scan data if the predection succeeded
            RSSIData scanData;
            scanData.label = locationLabel;
            for (int j = 0; j < TOTAL_APS; ++j) {
                scanData.RSSIs[j] = rssiPointToPredict[j];
            }

            dataSet.push_back(scanData);
        }
    }

    return validateScanAccuracy(predectionSuccess);
}

bool validateScanAccuracy(int predectionSuccess) {
    //Ask user approval for scan accuracy
    if(predectionSuccess >= VALIDATION_PASS_THRESHOLD) {
        scanAccuracy = (predectionSuccess/SCAN_VALIDATION_SAMPLE_SIZE)*100;
        Serial.println("Scanning Phase: scan accuracy at:" + locationToString(currentScanningLabel) + 
                       " predicted to be: " + String(predectionSuccess) + "%");
        return promptUserAccuracyApprove();
    }
    return false;
}

void scanStaticRSSI() {
    //start scanning - make 15 scan, each scan contain 3 samples and calculate EMA
    for (int scan = 0; scan < SCAN_BATCH_SIZE; ++scan) {
        int accumulatedRSSIs[TOTAL_APS];
        resetRssiBuffer(accumulatedRSSIs);
        for(int sample = 0; sample < SAMPLE_PER_SCAN_BATCH; ++sample) {
            int n = WiFi.scanNetworks();
            for (int j = 0; j < n; ++j) {
                String ssid = WiFi.SSID(j);
                int rssi = WiFi.RSSI(j);
                for (int k = 0; k < TOTAL_APS; ++k) {
                    if (ssid.equals(anchorSSIDs[k])) {
                        accumulatedRSSIs[k] = applyEMA(accumulatedRSSIs[k], rssi);
                    }
                }
            }
            WiFi.scanDelete();
            delay(SCAN_DELAY_MS);
        }

        //Add each scan to data set
        RSSIData scanData;
        scanData.label = locationLabel;
        for (int j = 0; j < TOTAL_APS; ++j) {
            scanData.RSSIs[j] = accumulatedRSSIs[j];
        }
        dataSet.push_back(scanData);
    }
}

int* createRSSIScanToMakePredection() {
    int accumulatedRSSIs[TOTAL_APS];
    resetRssiBuffer(accumulatedRSSIs);
    for(int sample = 0; sample < SAMPLE_PER_SCAN_BATCH; ++sample) {
        int n = WiFi.scanNetworks();
        for (int j = 0; j < n; ++j) {
            String ssid = WiFi.SSID(j);
            int rssi = WiFi.RSSI(j);
            for (int k = 0; k < TOTAL_APS; ++k) {
                if (ssid.equals(anchorSSIDs[k])) {
                    accumulatedRSSIs[k] = applyEMA(accumulatedRSSIs[k], rssi);
                }
            }
        }
        WiFi.scanDelete();
        delay(SCAN_DELAY_MS);
    }

    return accumulatedRSSIs;
}