#include "validationPhase.h"
#include "predictionPhase.h"
#include "userUI.h"
#include "scanning.h"
#include "rssiScanner.h"
#include "tofScanner.h"

static bool validatedLabels[NUMBER_OF_LABELS];

void runValidationPhase() {
    Serial.println();
    Serial.println("=============== Validation Phase Started ===============");

    for (int i = 0; i < NUMBER_OF_LABELS; i++) {
        Label label = promptLocationSelection();
        currentScanningLabel = label;

        Serial.println();
        Serial.println("Selected Label: " + String(labelToString(label)));
        Serial.println(">> Press Enter to start validation...");
        while (!Serial.available()) delay(50);
        Serial.read();  // consume newline

        Serial.println("Validation Phase: Starting validation at " + String(labelToString(label)));
        startLabelValidationSession();
    }

    Serial.println();
    Serial.println("=============== Validation Phase Completed ===============");

    printFinalValidationSummary(); 
}

void startLabelValidationSession() {
    int retryCount = 1;
    bool approved = false;

    while (retryCount <= MAX_RETRIES) {
        Serial.println("Validation Phase: Attempt " + String(retryCount) + " at " + String(labelToString(currentScanningLabel)));
        approved = validateScanAccuracy();

        if (approved) {
            Serial.println("Validation Phase: scan completed successfully at: " + String(labelToString(currentScanningLabel)));
            validatedLabels[currentScanningLabel] = true;
            return;
        }

        Serial.println("Validation Phase: accuracy insufficient, retrying scan... (" + String(retryCount) + "/" + String(MAX_RETRIES) + ")");
        retryCount++;
    }

    Serial.println("Validation Phase: failed after max retries at: " + String(labelToString(currentScanningLabel)));
    validatedLabels[currentScanningLabel] = false;
}

void printFinalValidationSummary() {
    Serial.println();
    Serial.println("=============== Validation Summary ===============");
    for (int i = 0; i < NUMBER_OF_LABELS; ++i) {
        String result = validatedLabels[i] ? "VALIDATED" : "NOT VALIDATED";
        Serial.println("Label " + String(i) + " - " + String(labelToString(i)) + ": " + result);
    }

    if (!failures.empty()) {
        Serial.println();
        Serial.println("-------- Failed Label Details --------");
        for (const auto& failure : failures) {
            Serial.println("Label " + String(labelToString(failure.label)) + " → Reason: " + failure.reason.c_str());
        }
    }

    Serial.println("==================================================");
    Serial.println();
}

bool validateScanAccuracy() {
    int matchesRSSI = -1;
    int matchesTOF = -1;

    Serial.println("Validation Phase: Running predictions...");

    switch (currentSystemState) {
        case STATIC_RSSI:
            Serial.println("Validation Mode: STATIC_RSSI");
            matchesRSSI = computeRSSIPredictionMatches();
            break;
        case STATIC_RSSI_TOF:
            Serial.println("Validation Mode: STATIC_RSSI_TOF");
            matchesRSSI = computeRSSIPredictionMatches();
            matchesTOF  = 0;
            matchesTOF  = computeTOFPredictionMatches();
            break;
        case STATIC_DYNAMIC_RSSI:
            Serial.println("Validation Mode: STATIC_DYNAMIC_RSSI");
            matchesRSSI = computeRSSIPredictionMatches();
            break;
        case STATIC_DYNAMIC_RSSI_TOF:
            Serial.println("Validation Mode: STATIC_DYNAMIC_RSSI_TOF");
            matchesRSSI = computeRSSIPredictionMatches();
            matchesTOF  = 0;
            matchesTOF  = computeTOFPredictionMatches();
            break;
        default:
            Serial.println("Validation Phase: Unknown system state.");
            return false;
    }

    int totalMatches = matchesRSSI + matchesTOF;
    int totalAttempts = (matchesTOF > -1) ? 2 * NUM_OF_VALIDATION_SCANS : NUM_OF_VALIDATION_SCANS;
    accuracy = (100 * totalMatches) / totalAttempts;

    Serial.println("Validation Phase: Accuracy = " + String(accuracy) +
                   "% (" + String(totalMatches) + "/" + String(totalAttempts) + " correct)");

    bool valid = (accuracy >= VALIDATION_PASS_THRESHOLD);
    return valid && promptUserAccuracyApprove();
}

bool isBackupDataSetRelevant(void) {
    Serial.println("Validation Check: Starting backup dataset relevance validation...");

    // Basic dataset size check for KNN feasibility
    if ((currentSystemState == STATIC_RSSI_TOF || currentSystemState == STATIC_DYNAMIC_RSSI_TOF) &&
         tofDataSet.size() < 0.5 * MIN_VALID_DATA_SET_SIZE) {
        Serial.println("Validation Check: TOF dataset too small for combined state.");
        return false;
    }

    if (rssiDataSet.size() < MIN_VALID_DATA_SET_SIZE) {
        Serial.println("Validation Check: RSSI dataset too small.");
        return false;
    }

    unsigned int numOfLabelInRSSI[NUMBER_OF_LABELS] = {0};
    unsigned int numOfLabelInTOF[NUMBER_OF_LABELS] = {0};

    bool isDataSetValid = false;
    int sizeOfDataSet = dataSet.size();

    for (const RSSIData &data : rssiDataSet) {
        numOfLabelInRSSI[data.label]++;
    }

    for (const TOFData &data : tofDataSet) {
        numOfLabelInTOF[data.label]++;
    }

    for (int i = 0; i < NUMBER_OF_LABELS; ++i) {

        Serial.println("Validation Check: Label " + String(i) + " - " + labelToString(i));
        Serial.println("  RSSI samples: " + String(numOfLabelInRSSI[i]));
        Serial.println("  TOF samples : " + String(numOfLabelInTOF[i]));

        currentLabel = (Label)i;
        if (numOfLabelInRSSI[i] >= MIN_DATA_PER_LABEL_SIZE&& numOfLabelInTOF[i] >= 0.5 * MIN_DATA_PER_LABEL_SIZE && validateScanAccuracy()) {
            reuseFromSD[i] = true;
            isDataSetValid = true;
            Serial.println("Valid scan data — will reuse backup for: " + String(labelToString(i)));
        }
        else {
            Serial.println("Backup dataset not sufficient for: " + String(labelToString(i)));        }
    }

    if (isDataSetValid) {
        Serial.println("Validation Check: At least one label has valid backup data.");
    } else {
        Serial.println("Validation Check: No label has sufficient backup data.");
    }

    return isDataSetValid;
    
}
