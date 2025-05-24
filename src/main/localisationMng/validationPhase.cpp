#include "validationPhase.h"
#include "predictionPhase.h"
#include "userUI.h"
#include "scanning.h"
#include "rssiScanner.h"
#include "tofScanner.h"

#define VALIDATION_MAX_ATTEMPTS 3

static bool validatedLabels[NUMBER_OF_LABELS] = {false};

struct ValidationFailure {
    Label label;
    const char* reason;
};

static std::vector<ValidationFailure> failedValidations;


void startLabelValidationSession() {
    Serial.println("=== Validation Phase Started ===");

<<<<<<< HEAD
    while (true) {
        Serial.println("\n[Validation] Walk to any label...");
        delay(3000);

        bool approved = false;
        Label predicted = NOT_ACCURATE;

        for (int attempt = 1; attempt <= VALIDATION_MAX_ATTEMPTS; ++attempt) {
            double rssiInput[NUMBER_OF_ANCHORS] = {0};
            double  tofInput[NUMBER_OF_RESPONDERS] = {0};

            if (currentSystemState == STATIC_RSSI || currentSystemState == STATIC_DYNAMIC_RSSI ||
                currentSystemState == STATIC_RSSI_TOF || currentSystemState == STATIC_DYNAMIC_RSSI_TOF) {
                int* rssi = createRSSIScanToMakePrediction();
                for (int i = 0; i < NUMBER_OF_ANCHORS; ++i)
                    rssiInput[i] = rssi[i];
                preparePoint(rssiInput);
            }

            if (currentSystemState == STATIC_RSSI_TOF || currentSystemState == STATIC_DYNAMIC_RSSI_TOF) {
                createTOFScanToMakePrediction(tofInput);
            }

            predicted = (currentSystemState == STATIC_RSSI || currentSystemState == STATIC_DYNAMIC_RSSI)
                        ? rssiPredict(rssiInput)
                        : tofPredict(tofInput);

            Serial.printf("[Attempt %d] Predicted label: %s\n", attempt, labelToString(predicted));
            int userChoice = promptValidationApprovalOrSkip();

            if (userChoice == 1) {
                validatedLabels[predicted] = true;
                Serial.println("Validation confirmed.");
                approved = true;
                break;
            } else if (userChoice == 9) {
                printValidationWarning(predicted, "User manually skipped");
                failedValidations.push_back({predicted, "User skipped manually"});
                approved = false;
                break;
            } else {
                Serial.println(" Rejected. Retrying...");
            }
        }

        if (!approved) {
            if (std::none_of(failedValidations.begin(), failedValidations.end(),
                             [predicted](const ValidationFailure& f) { return f.label == predicted; })) {
                printValidationWarning(predicted, "Repeated incorrect predictions or unstable signal");
                failedValidations.push_back({predicted, "Unstable prediction or environment"});
            }
        }

        if (!promptVerifyScanCoverageAtAnotherLabel()) {
=======
    bool approved = false;
    while (int retryCount = 1; attempt <= VALIDATION_MAX_ATTEMPTS;) {
        Serial.println("validating scan accuracy...");
        approved = validateScanAccuracy();
        if (validScan) {
            Serial.printf("Validation Phase: scan completed successfully at: %s\n", labelToString(label));
>>>>>>> 53f8e7f (change validation function)
            break;
        }
            
        Serial.println("Validation Phase: accuracy insufficient, retrying scan...");
        retryCount++;
    }
    if (!approved) {
        Serial.printf("Unstable prediction or environment, validation phase failed for label: %s\n", labelToString(label));
    }

     printFinalValidationSummary(validatedLabels, failedValidations);
     Serial.println("=== Validation Phase Completed ===");
}

void printFinalValidationSummary(const bool validatedLabels[], const std::vector<ValidationFailure>& failures) {
    Serial.println("\n======= VALIDATION SUMMARY =======");
    for (int i = 0; i < NUMBER_OF_LABELS; ++i) {
        Serial.printf("Label %d - %s: %s\n", i, labelToString(i), validatedLabels[i] ? " VALIDATED" : " NOT VALIDATED");
    }

    if (!failures.empty()) {
        Serial.println("\n FAILED VALIDATIONS:");
        for (const auto& f : failures) {
            Serial.printf("Label %s → %s\n", labelToString(f.label), f.reason);
        }
    }

    Serial.println("==================================\n");
}

/**
 * @brief Validates the scan results by checking how many predictions match the label.
 *        Uses both RSSI and TOF modules if enabled. If failed, offers fallback.
 */
bool validateScanAccuracy() {
    int matchesRSSI = 0;
    int matchesTOF = 0;
    bool combinedOK = false;

    // Run RSSI and/or TOF predictions depending on system state
    switch (Enablements::currentSystemState) {
        case STATIC_RSSI:
            matchesRSSI = computeRSSIPredictionMatches();
            Serial.printf("validating according to static rssi");
            break;

        case STATIC_RSSI_TOF:
            matchesRSSI = computeRSSIPredictionMatches();
            matchesTOF = computeTOFPredictionMatches();
            Serial.printf("validating according to static rssi, tof");
            break;

        case STATIC_DYNAMIC_RSSI:
            matchesRSSI = computeRSSIPredictionMatches();
            Serial.printf("validating according to static rssi, dynamic rssi");
            break;

        case STATIC_DYNAMIC_RSSI_TOF:
            matchesRSSI = computeRSSIPredictionMatches();
            matchesTOF = computeTOFPredictionMatches();
            Serial.printf("validating according to static rssi, dynamic rssi, tof");
            break;

        default:
            break;
    }

    int totalMatches = matchesRSSI + matchesTOF;
    int totalAttempts = (matchesTOF > 0) ? 2 * SCAN_VALIDATION_SAMPLE_SIZE : SCAN_VALIDATION_SAMPLE_SIZE;
    scanAccuracy = (100 * totalMatches) / totalAttempts;

    Serial.printf("accuracy = %d%% (%d/%d correct predictions)\n",
                  scanAccuracy, totalMatches, totalAttempts);

    combinedOK = (scanAccuracy >= VALIDATION_PASS_THRESHOLD);

    return combinedOK && promptUserAccuracyApprove();
}

