/**
 * @file tofScanner.cpp
 * @brief ToF (Time-of-Flight) scanning using Wi-Fi FTM sessions with registered responders.
 *
 * Measures round-trip time and computes distance for each known responder.
 * Each scan is handled via ESP-IDF FTM events and logged to internal structures.
 * Supports both batch scanning (`performTOFScan`) and single scan execution.
 */

#include "tofScanner.h"
#include "core/dataManaging/data.h"
#include "core/prediction/predictionPhase.h"

static bool registered = false;
static bool inProcess  = false;
static bool scanComplete = false;


//Handler function used as call back function when responder sends back to initiator
static void tofReportHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    //esp passes data about the session in event_data 
    //(MAC address of the responder, Which responder this is (index), Round-trip time in nanoseconds, 
    //Estimated distance in millimeters, Whether it worked or failed)
    auto* report = (wifi_event_ftm_report_t*)event_data;
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
        SaveBufferedData::scanner = TOF;
        SaveBufferedData::lastN++;
        
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

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    if(!registered && !inProcess) {
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &tofReportHandler, NULL);
        registered = true;
        inProcess  = true;
    }

    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        wifi_ftm_initiator_cfg_t cfg = {
            .frm_count = 16,
            .burst_period = 2,
        };

        cfg.channel = 1;
        memcpy(cfg.resp_mac , responderMacs[0], sizeof(cfg.resp_mac));

        scanComplete = false;
        accumulatedTOFs[i] = TOF_DEFAULT_DISTANCE_CM;

        esp_err_t err = esp_wifi_ftm_initiate_session(&cfg);

        if (err != ESP_OK) {
            LOG_ERROR("TOF", "Failed to initiate session for responder: %s", tofSSIDs[i].c_str());
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
            LOG_INFO("TOF", "TOF scan completed after: %ul", (duration-start));
        }
    }

    if(inProcess) {
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_FTM_REPORT, &tofReportHandler);
    }

    TOFData scanData;
    scanData.label = currentLabel;
    for (int i = 0; i < NUMBER_OF_RESPONDERS; i++) {
        scanData.TOFs[i] = accumulatedTOFs[i];
    }
    return scanData;
}