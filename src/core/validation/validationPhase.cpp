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

        promptUserLocationLabel();
        startLabelValidationSession();

        if (!validForPredection[currentLabel]) {
            delay_ms(USER_PROMPTION_DELAY);
            LOG_ERROR("VALIDATE", "Predection failure at: %s", labels[currentLabel]);
            bool _rescan = promptUserRescanAfterInvalidation();
            if(_rescan) {
                LOG_INFO("VALIDATE", "Rescanning...");

                if(SystemSetup::logLevel < LogLevel::LOG_LEVEL_DEBUG) {
                    promptUserShowDebugLogs();
                }

                rescan();
            }
            else {
                 promptUserAbortOrContinue();
            }
        }
    }

    printFinalValidationSummary(); 
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
            LOG_INFO("VALIDATION", "Data set validation session for : %s completed. Accuracy detected : %d", labels[currentLabel], getAccuracy());
            validForPredection[currentLabel] = true;
            return;
        }

        LOG_ERROR("VALIDATE", "Predicted location: %s - Expected location: %s", labels[predicted], labels[currentLabel]);
        delay_ms(DELAY_BETWEEN_PHASES);

        retry = promptUserRetryPrediction();

        if(retryCount >= VALIDATION_MAX_ATTEMPTS) {
            LOG_ERROR("VALIDATE", "Data set seems invalid for: %s", labels[predicted]);
            validForPredection[currentLabel] = false;
            return;
        }

        delay_ms(DELAY_BETWEEN_PHASES);
    }
}

void printFinalValidationSummary() {
    LOG_INFO("VALIDATE", " ");
    LOG_INFO("VALIDATE", "=============== Validation Summary ===============");

    for (int i = 0; i < LABELS_COUNT; ++i) {
        currentLabel = (Label)i;
        const char* result = validForPredection[i] ? "VALIDATED" : "NOT VALIDATED";

        LOG_INFO("VALIDATE", "Label %d - %s: %s, with accuracyL %d", i, labels[i], result, getAccuracy());
    }

    currentLabel = (Label)LABELS_COUNT;

    LOG_INFO("VALIDATE", "==================================================");
    LOG_INFO("VALIDATE", " ");
}