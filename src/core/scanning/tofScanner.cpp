#include "scanningPhase.h"
#include "core/utils/platform.h"
#include "core/utils/utilities.h"  
#include "core/ui/logger.h"
#include "tofScanner.h"
#include "core/dataManaging/data.h"
#include "core/prediction/predictionPhase.h"
#include "esp_wifi.h"

static bool registered = false;
static bool inProcess  = false;
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

    resetTOFScanBuffer();
    // register a callback function (ftmReportHandler) to listen for FTM result events.
    if(!registered && !inProcess) {
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &tofReportHandler, NULL);
        registered = true;
        inProcess  = true;
    }

    for(int scan = 0; scan < TOF_SCAN_BATCH_SIZE ; scan++) {
        
        TOFData scanData = createSingleTOFScan();
        saveData(scanData);
        BufferedData::scanner = TOF_;
        BufferedData::lastN++;
        
        LOG_DEBUG("TOF", "Scan %d for label %s", scan + 1, labels[currentLabel]);

        for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
            LOG_DEBUG("TOF", "SSID[%s], Diatance = %d", tofSSIDs[i] , accumulatedTOFs[i]);
        }
    }
    esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &tofReportHandler);
    registered = false;
    inProcess  = false;

    doneCollectingData();
}

TOFData createSingleTOFScan() {

    if(!registered && !inProcess) {
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &tofReportHandler, NULL);
        registered = true;
        inProcess  = true;
    }

    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        wifi_ftm_initiator_cfg_t cfg = {
            .frm_count = 16,
            .burst_period = 2
        };

        cfg.channel = 0 ; // Wi-fi channel to use for FTM - 0 since it's unknown

        scanComplete = false;
        accumulatedTOFs[i] = TOF_DEFAULT_DISTANCE_CM;

        esp_err_t err = esp_wifi_ftm_initiate_session(&cfg);

        if (err != ESP_OK) {
            LOG_ERROR("TOF", "Failed to initiate session for responder: %s", tofSSIDs[i]);
            continue;
        }

        unsigned long start    = millis_since_boot();
        unsigned long duration = millis_since_boot();
        while (!scanComplete && duration - start < 300) {
            delay_ms(TOF_SCAN_DELAY_MS);
            duration = millis_since_boot();
        }
        if(!scanComplete) {
             LOG_ERROR("TOF", "TOF scan timeout, aborting current scanning session");
        } 
        else {
            LOG_ERROR("TOF", "TOF scan completed after: %l", duration-start);
        }
    }

    if(inProcess) {
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &tofReportHandler);
    }
}

/*
int computeTOFPredictionMatches() {
    int matches = 0;

    for (int v = 0; v < TOF_SCAN_SAMPLE_PER_BATCH; ++v) {
        createSingleTOFScan();
        Label predicted = tofPredict();

        if (predicted == currentLabel) {
            matches++;

            TOFData data;
            data.label = currentLabel;
            for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
                data.TOFs[i] = accumulatedTOFs[i];
            }
            tofDataSet.push_back(data);
            saveData(data);
        }

        LOG_DEBUG("TOF", "[VALIDATION] #%d: Predicted %s | Actual %s",
            v + 1, labels[predicted], labels[currentLabel]);
    }

    
    return matches;
}

*/