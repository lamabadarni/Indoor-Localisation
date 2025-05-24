#include "scanning.h"

static double lastTOFScan[NUMBER_OF_RESPONDERS];
static bool scanComplete = false;

//Handler function used as call back function when responder sends back to initiator
static void tofReportHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    //esp passes data about the session in event_data 
    //(MAC address of the responder, Which responder this is (index), Round-trip time in nanoseconds, 
    //Estimated distance in millimeters, Whether it worked or failed)
    auto* report = (wifi_event_ftm_report_t*)event_data;
    int i = report->peer_idx;
    if (i >= NUMBER_OF_RESPONDERS) return;

    if (report->status == FTM_STATUS_SUCCESS) {
        //Convert distance from millimeters to centimeters.
        double cm = report->distance_mm / 10.0;
    } else {
        // If failed, set a default value (e.g., 1500 cm)
        lastTOFScan[idx] = 1500.0;
    }

    scanComplete = true;
}


void performTOFScan() {
    // register a callback function (ftmReportHandler) to listen for FTM result events.
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &tofReportHandler, NULL);
    memset(currentTOFs, 0, sizeof(currentTOFs));

    for(int scan = 0; scan < TOF_SCAN_BATCH_SIZE ; scan++) {
        TOFData scanData;
        scanData.label = currentLabel;

        for (int i = 0; i < NUMBER_OF_RESPONDERS; i++) {
            wifi_ftm_initiator_cfg_t config = {
                .frm_count = 16, 
                .burst_period = 2
            };

            scanComplete = false;
            // in case of failure, set a default value
            lastTOFScan[i] = 1500.0;

            esp_err_t err = esp_wifi_ftm_initiate_session(responderMacs[i], &cfg);
            if (err != ESP_OK) {
                Serial.printf("[TOF] Session failed for responder %d (%s)\n", i, esp_err_to_name(err));
                continue;
            }

            unsigned long start = millis();
            while (!scanComplete && millis() - start < 300) {
                delay(10);
            }

            data.TOFs[i] = lastTOFScan[i];
        }

        //Should Save to CSV !!!!!
        tofDataSet.push_back(data);
       saveTOFScan(data);


        Serial.printf("[TOF] Scan %d for label %s: ", s + 1, labelToString(currentLabel));
        for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
            Serial.printf("%.1f  ", data.TOFs[i]);
        }
        Serial.println();
    }

    esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &tofReportHandler);
}

void createTOFScanToMakePrediction(double out[NUMBER_OF_RESPONDERS]) {
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &tofReportHandler, NULL);

    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        wifi_ftm_initiator_cfg_t cfg = {
            .frm_count = 16,
            .burst_period = 2
        };

        scanComplete = false;
        // in case of failure, set a default value
        lastTOFScan[i] = 1500.0;

        esp_err_t err = esp_wifi_ftm_initiate_session(responderMacs[i], &cfg);
        if (err != ESP_OK) {
            Serial.printf("[TOF] FTM failed during prediction for responder %d (%s)\n", i, esp_err_to_name(err));
            continue;
        }

        unsigned long start = millis();
        while (!scanComplete && millis() - start < 300) {
            delay(10);
        }

        out[i] = lastTOFScan[i];
    }

    esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &tofReportHandler);
}


int computeTOFPredictionMatches() {
    int matches = 0;

    for (int v = 0; v < SCAN_VALIDATION_SAMPLE_SIZE; ++v) {
        double scan[NUMBER_OF_RESPONDERS];
        createTOFScanToMakePrediction(scan);
        Label predicted = tofPredict(scan);

        if (predicted == currentLabel) {
            matches++;

            TOFData data;
            data.label = currentLabel;
            for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
                data.TOFs[i] = scan[i];
            }
            tofDataSet.push_back(data);
                   saveTOFScan(data);

        }

        Serial.printf("[TOF VALIDATION] #%d: Predicted %s | Actual %s\n",
                      v + 1, labelToString(predicted), labelToString(currentLabel));
    }

    return matches;
}