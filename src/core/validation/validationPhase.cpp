/**
 * @file validationPhase.cpp
 * @brief Implements the validation phase where the system predicts user location
 *        and confirms correctness via user input.
 *
 * Each label undergoes one or more prediction attempts.
 * The user confirms prediction results or chooses to rescan or abort.
 *
 * Key Flow:
 * - Run full validation phase (`runValidationPhase`)
 * - Validate individual labels (`startLabelValidationSession`)
 */


#include "validationPhase.h"
#include "core/scanning/scanningPhase.h"
#include "core/prediction/predictionPhase.h"
#include "core/dataManaging/data.h"

void runValidationPhase() {
    
    delay_ms(DELAY_BETWEEN_PHASES);

    LOG_INFO("VALIDATE", "=============== Validation Phase Started ===============");
    LOG_INFO("VALIDATE", "In this phase, the system will predict your current location.");
    LOG_INFO("VALIDATE", "You will be asked to confirm whether the prediction is correct.");
    LOG_INFO("VALIDATE", "Validation phase will continue until running validation phase at all labels or you decide to abort.");

    setValidForPredection();

    while (!shouldAbort) {
        LOG_INFO("VALIDATE", "Please stand still at a label, press enter when you're ready");

        promptUserLocationLabelForScan();
        startLabelValidationSession();

        if (!validForPredection[currentLabel]) {
            delay_ms(USER_PROMPTION_DELAY);
            LOG_ERROR("VALIDATE", "Predection failure at: %s", labels[currentLabel].c_str());
            bool rescanD = promptUserRescanAfterInvalidation();
            if(rescanD) {
                LOG_INFO("VALIDATE", "Rescanning...");

                if(SystemSetup::logLevel < LogLevel::LOG_LEVEL_DEBUG) {
                    promptUserShowDebugLogs();
                }

                rescan();
            }
            promptUserProceedToNextLabel();
        }
    }

    LOG_INFO("VALIDATE", " ");
    LOG_INFO("VALIDATE", "=============== Validation Summary ===============");

    for (int i = 0; i < LABELS_COUNT; ++i) {
        currentLabel = (Label)i;
        const char* result = validForPredection[i] ? "VALIDATED" : "NOT VALIDATED";

        LOG_INFO("VALIDATE", "Label %d - %s: %s" , labels[i].c_str() , result);
    }

    currentLabel = (Label)LABELS_COUNT;

    LOG_INFO("VALIDATE", "==================================================");
    LOG_INFO("VALIDATE", " ");
}

void startLabelValidationSession() {
    bool retry  = true;
    int retryCount = 0;

    while (retry) {
        retry = false; 
        retryCount++;

        LOG_INFO("VALIDATE", "Attempt %d", retryCount);

        createSingleScan();
        Label predicted = predict();

        if(predicted == currentLabel) {
            LOG_INFO("VALIDATION", "Data set validation session for : %s completed. ", labels[currentLabel].c_str());
            validForPredection[currentLabel] = true;
            SaveBufferedData::scanner = DYNAMIC_RSSI;
            SaveBufferedData::lastN++;
            DynamicRSSIData rssiScanData;
            rssiScanData.label = predicted;
            for(int i=0; i<NUMBER_OF_DYNAMIC_APS; i++) {
                rssiScanData.RSSIs[i] = accumulatedDynamicRSSIs[i];
            }
            DynamicMacData macData;
            macData.label = predicted;
            memcpy(macData.macAddresses, accumulatedMacAddresses, sizeof(macData.macAddresses));
            saveData(macData, rssiScanData);
            return;
        }

        LOG_ERROR("VALIDATE", "Predicted location: %s - Expected location: %s", labels[predicted].c_str(), labels[currentLabel].c_str());
        delay_ms(DELAY_BETWEEN_PHASES);


        if(retryCount >= VALIDATION_MAX_ATTEMPTS) {
            LOG_ERROR("VALIDATE", "Data set seems invalid for: %s", labels[predicted].c_str());
            break;
        }

        retry = promptUserRetryValidation();

        delay_ms(DELAY_BETWEEN_PHASES);
    }

    validForPredection[currentLabel] = false;
    return;
}