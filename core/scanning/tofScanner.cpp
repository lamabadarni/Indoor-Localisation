#include <scanning.h>
#include <string.h>
#include <Core/utils/utilities.h>  

#include <tofScanner.h>
#include "esp_wifi.h"

static bool scanComplete = false;

//Handler function used as call back function when responder sends back to initiator
static void tofReportHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    //esp passes data about the session in event_data 
    //(MAC address of the responder, Which responder this is (index), Round-trip time in nanoseconds, 
    //Estimated distance in millimeters, Whether it worked or failed)
    auto* report = (wifi_event_ftm_report_t*)event_data;
    uint8_t* mac = report->peer_mac;
    int i = NUMBER_OF_RESPONDERS + 1;

    for (int j = 0; j < NUMBER_OF_RESPONDERS; ++j) {
        if (memcmp(report->peer_mac, responderMacs[j], 6) == 0) {
            i = j;
        }
    }

    if (i >= NUMBER_OF_RESPONDERS) return;

    if (report->status == FTM_STATUS_SUCCESS) {
        //Convert distance from millimeters to centimeters.
        // Speed of light in meters per second
        const double speedOfLight = 299792458.0;
        
        // rtt_raw is in nanoseconds
        double distance_meters = (report->rtt_raw / 1e9) * (speedOfLight / 2.0);  // Divide by 2 for RTT
        double distance_cm = distance_meters * 100.0;
        accumulatedTOFs[i] = distance_cm;
    } else {
        accumulatedTOFs[i] = TOF_DEFAULT_DISTANCE_CM;
    }

    scanComplete = true;
}

void performTOFScan() {
    // register a callback function (ftmReportHandler) to listen for FTM result events.
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &tofReportHandler, NULL);

    for(int scan = 0; scan < TOF_SCAN_BATCH_SIZE ; scan++) {
        TOFData scanData;
        scanData.label = currentLabel;

        for (int i = 0; i < NUMBER_OF_RESPONDERS; i++) {
            wifi_ftm_initiator_cfg_t config = {
                .frm_count = 16, 
                .burst_period = 2
            };

            scanComplete = false;
            accumulatedTOFs[i] = TOF_DEFAULT_DISTANCE_CM;

            esp_err_t err = esp_wifi_ftm_initiate_session(responderMacs[i], &config);
            if (err != ESP_OK) {
                LOG_WARN("TOF", "Session failed for responder %d (%s)", i, esp_err_to_name(err));
                continue;
            }

            unsigned long start = millis_since_boot();
            while (!scanComplete && millis_since_boot() - start < 300) {
                delay_ms(TOF_SCAN_DELAY_MS);
            }

            scanData.TOFs[i] = accumulatedTOFs[i];
        }
        tofDataSet.push_back(scanData);
        saveTOFScan(scanData);


        LOG_INFO("TOF", "Scan %d for label %s", scan + 1, labelToString(currentLabel));
        LOG_INFO("TOF", "TOF[%d] = %.1f cm", i, scanData.TOFs[i]);
        
        Serial.println(output);
    }

    esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &tofReportHandler);
}

void createTOFScanToMakePrediction() {
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &tofReportHandler, NULL);

    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        wifi_ftm_initiator_cfg_t cfg = {
            .frm_count = 16,
            .burst_period = 2
        };

        scanComplete = false;
        accumulatedTOFs[i] = TOF_DEFAULT_DISTANCE_CM;

        esp_err_t err = esp_wifi_ftm_initiate_session(responderMacs[i], &cfg);
        if (err != ESP_OK) {
            LOG_INFO("TOF", "[VALIDATION] #%d: Predicted %s | Actual %s", v + 1,
                labelToString(predicted), labelToString(currentLabel));

            continue;
        }

        unsigned long start = millis();
        while (!scanComplete && millis() - start < 300) {
            delay_ms(TOF_SCAN_DELAY_MS);
        }
    }

    esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &tofReportHandler);
}


int computeTOFPredictionMatches() {
    int matches = 0;

    for (int v = 0; v < SCAN_VALIDATION_SAMPLE_SIZE; ++v) {
        createTOFScanToMakePrediction();
        Label predicted = tofPredict();

        if (predicted == currentLabel) {
            matches++;

            TOFData data;
            data.label = currentLabel;
            for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
                data.TOFs[i] = accumulatedTOFs[i];
            }
            tofDataSet.push_back(data);
            saveTOFScan(data);

        }

        Serial.println("[TOF VALIDATION] #" + String(v + 1) + ": Predicted " + String(labelToString(predicted)) + 
                       " | Actual " + String(labelToString(currentLabel)));

    }
    return matches;
}