#include <validationPhase.h>
#include <predictionPhase.h>
#include <userUI.h>
#include <scanning.h>
#include <rssiScanner.h>
#include <tofScanner.h>
#include "utils/logger.h"

static bool validatedLabels[NUMBER_OF_LABELS];

void runValidationPhase() {
    LOG_INFO("UI", " ");
    LOG_INFO("UI", "=============== Validation Phase Started ===============");

    for (int i = 0; i < NUMBER_OF_LABELS; i++) {
        currentLabel = promptLocationLabel();

        LOG_INFO("UI", " ");
        LOG_INFO("VALIDATE", "Selected label: %s", labelToString(currentLabel));
        LOG_INFO("UI", ">> Press Enter to start validation...");
        while (!Serial.available()) delay_ms(50);
        readCharFromUser();  // consume newline

        LOG_INFO("VALIDATE", "Starting validation at: %s", labelToString(currentLabel));
        startLabelValidationSession();
    }

    LOG_INFO("UI", " ");
    LOG_INFO("UI", "=============== Validation Phase Completed ===============");

    printFinalValidationSummary(); 
}

void startLabelValidationSession() {
    int retryCount = 1;
    bool approved = false;

    while (retryCount <= MAX_RETRIES) {
        LOG_INFO("VALIDATE", "Attempt %d at %s", retryCount, labelToString(currentLabel));
        approved = validateScanAccuracy();

        if (approved) {
            LOG_INFO("VALIDATE", "Scan completed successfully at: %s", labelToString(currentLabel));
            validatedLabels[currentLabel] = true;
            return;
        }

        LOG_WARN("VALIDATE", "Accuracy insufficient, retrying... (%d/%d)", retryCount, MAX_RETRIES);
        retryCount++;
    }

    LOG_ERROR("VALIDATE", "Failed after max retries at: %s", labelToString(currentLabel));
    validatedLabels[currentLabel] = false;
}

void printFinalValidationSummary() {
    LOG_INFO("UI", " ");
    LOG_INFO("UI", "=============== Validation Summary ===============");
    for (int i = 0; i < NUMBER_OF_LABELS; ++i) {
        const char* result = validatedLabels[i] ? "VALIDATED" : "NOT VALIDATED";
        LOG_INFO("VALIDATE", "Label %d - %s: %s", i, labelToString((Label)i), result);
    }
    LOG_INFO("UI", "==================================================");
    LOG_INFO("UI", " ");
}

bool validateScanAccuracy() {
    int matchesRSSI = -1;
    int matchesTOF = -1;

    LOG_INFO("VALIDATE", "Running predictions...");

    switch (currentSystemState) {
        case STATIC_RSSI:
            LOG_INFO("VALIDATE", "Mode: STATIC_RSSI");
            matchesRSSI = computeRSSIPredictionMatches();
            break;
        case STATIC_RSSI_TOF:
            LOG_INFO("VALIDATE", "Mode: STATIC_RSSI_TOF");
            matchesRSSI = computeRSSIPredictionMatches();
            matchesT
    }
}