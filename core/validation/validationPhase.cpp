
#include "validationPhase.h"
#include "../utils/platform.h"
#include "../utils/utilities.h"
#include "../prediction/predictionPhase.h"
#include "../ui/userUI.h"
#include "../scanning/scanningPhase.h"

static bool validatedLabels[LABELS_COUNT];

void runValidationPhase() {
    
    delay_ms(DELAY_BETWEEN_PHASES);

    LOG_INFO("VALIDATE", "=============== Validation Phase Started ===============");
    LOG_INFO("VALIDATE", "In this phase, the system will predict your current location.");
    LOG_INFO("VALIDATE", "You will be asked to confirm whether the prediction is correct.");
    LOG_INFO("VALIDATE", "Validation phase will continue until running validation phase at all labels or you decide to abort.");

    while( !shouldAbort && !forceNextPhase ) {
        LOG_INFO("VALIDATE", "Please stand still at a location");

        startLabelValidationSession();
        
        if (!validatedLabels[currentLabel]) {
            delay_ms(USER_PROMPTION_DELAY);
            userRescanAfterFailedInvalidation();
            LOG_INFO("VALIDATE", "Rescanning...");
            if!(SystemSetup::printDebugLogs) {
                userShowDebugLogsWhileRescan();
                performRescan();
            }
        }

        promotAbortOrContinue();
    }

    printFinalValidationSummary(); 
}

void startLabelValidationSession() {
    bool approved  = false;

    while (true) {
        LOG_INFO("VALIDATE", "Attempt %d", retryCount);

        RSSIData data = createSingleRSSIScan();
        currentLabel = rssiPredict();

        LOG_INFO("VALIDATE", "Predicted location: %s", labels[predicted]);

        approved =  userValidationApprove();

        if (approved) {
            LOG_INFO("VALIDATE", "User approved prediction at: %s", labels[predicted]);
            validatedLabels[currentLabel] = true;
            saveToBuffer(data);
            return;
        }
        else {
            LOG_WARN("VALIDATE", "User rejected prediction at: %s", labels[predicted]);
            if(!userValidationRetry()) {
                return;
            }
        }
        delay_ms(DELAY_BETWEEN_PHASES);
    }

    LOG_ERROR("VALIDATE", "Failed after max retries at: %s", labels[predicted]);
    validatedLabels[currentLabel] = false;
}

void printFinalValidationSummary() {
    LOG_INFO("VALIDATE", " ");
    LOG_INFO("VALIDATE", "=============== Validation Summary ===============");
    for (int i = 0; i < LABELS_COUNT; ++i) {
        const char* result = validatedLabels[i] ? "VALIDATED" : "NOT VALIDATED";
        LOG_INFO("VALIDATE", "Label %d - %s: %s", i, labelToString((Label)i), result);
    }
    LOG_INFO("VALIDATE", "==================================================");
    LOG_INFO("VALIDATE", " ");
}

bool validateScanAccuracy() {
    int matchesRSSI = -1;
    int matchesTOF = -1;

    LOG_INFO("VALIDATE", "Validating scan accuracy...");

    switch (currentSystemState) {
        case STATIC_RSSI:
            matchesRSSI = computeRSSIPredictionMatches();
            break;
        case STATIC_RSSI_TOF:
            matchesRSSI = computeRSSIPredictionMatches();
    }
}