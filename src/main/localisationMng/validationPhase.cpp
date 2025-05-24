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

bool isLocationDataValid(Label location) {
    return validatedLabels[location];
}

void runValidationPhase() {
    Serial.println("=== Validation Phase Started ===");

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
            break;
        }
    }

    printFinalValidationSummary(validatedLabels, failedValidations);
    Serial.println("=== Validation Phase Completed ===");
}

void printValidationWarning(Label label, const char* reason) {
    Serial.println(" Validation Warning:");
    Serial.printf("Label %s failed validation. Reason: %s\n", labelToString(label), reason);
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