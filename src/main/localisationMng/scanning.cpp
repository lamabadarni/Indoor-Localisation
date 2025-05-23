#include "scanning.h"

static int staticRSSIBufferToPredict[SCAN_VALIDATION_SAMPLE_SIZE][NUMBER_OF_ANCHORS];
static float tofBufferToPredict[SCAN_VALIDATION_SAMPLE_SIZE][NUMBER_OF_RESPONDERS]; 
static int scanAccuracy = 0;


static void resetRssiBuffer(int rssi[NUMBER_OF_ANCHORS]) {
    for(int i = 0; i < NUMBER_OF_ANCHORS; i++) {
        rssi[i] = DEFAULT_RSSI_VALUE;
    }
}

static void resetPredectionBuffers() {
    for(int i = 0; i < SCAN_VALIDATION_SAMPLE_SIZE; i++) {
        for(int j = 0; j < NUMBER_OF_ANCHORS j++) {
            staticRSSIBufferToPredict[i][j] = DEFAULT_RSSI_VALUE;
        }
    }
}

bool startLabelScanningSession(Label label) {
    int retryCount       = 0;
    bool validScan       = false;

    while(retryCount < MAX_RETRIES) {
        Serial.println("Scanning Phase: perfroming scan attempt #" + String(retryCount + 1) + " for: " locationToString(currentScanningLabel));
        collectMeasurements();
        Serial.println("Scanning Phase: scan attempt #" + String(retryCount + 1) + " checking scan validity");
        validScan = validateScanMeasurements();
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

int computePredictionMatches() {
    int predectionSuccess = 0;
    Point point;

    for(int sampleToPredict = 0; sampleToPredict < SCAN_VALIDATION_SAMPLE_SIZE; sampleToPredict++) {
        memcpy(point.RSSIs, staticRSSIBufferToPredict[i], sizeof(point.RSSIs));
        memcpy(point.TOFs, tofBufferToPredict[i], sizeof(point.TOFs));
        Label predictedLabel = predict(point);
        if(predictedLabel == currentScanningLabel) {
            predectionSuccess++;
        }
    }

    return validateScanAccuracy();
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
    //reset the scanning batch that will be used for predection
    resetPredectionBuffers(nullptr);
    //start scanning 
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

        int lastFiveScans = SCAN_BATCH_SIZE - scan;
        if(lastFiveScans <= 5) {
            memcpy(staticRSSIBufferToPredict[lastFiveScans - 1], accumulatedRSSIs, sizeof(point.RSSIs));
        }

        Data scanData;
        scanData.label = locationLabel;
        for (int j = 0; j < TOTAL_APS; ++j) {
            scanData.RSSIs[j] = accumulatedRSSIs[j];
        }
        dataSet.push_back(scanData);
    }
}