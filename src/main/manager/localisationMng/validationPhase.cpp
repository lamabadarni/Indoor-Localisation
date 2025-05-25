/**
 * @file validationPhase.cpp
 * @brief Validates system accuracy for each label using real-time predictions and user feedback, supports retry logic and dataset reuse check.
 * @author Lama Badarni
 */

#include <validationPhase.h>
#include <predictionPhase.h>
#include <userUI.h>
#include <scanning.h>
#include <rssiScanner.h>

static bool validatedLabels[NUMBER_OF_LABELS];

void runValidationPhase() {
    Serial.println();
    Serial.println("=============== Validation Phase Started ===============");

    for (int i = 0; i < NUMBER_OF_LABELS; i++) {
        currentLabel = promptLocationLabel();

        Serial.println();
        Serial.println("[VALIDATE] Selected Label: " + String(labelToString(currentLabel)));
        Serial.println("[VALIDATE] >> Press Enter to start validation...");
        while (!Serial.available()) delay(50);
        Serial.read();  // consume newline

        Serial.println("[VALIDATE] Starting validation at: " + String(labelToString(currentLabel)));
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
        Serial.println("[VALIDATE] Attempt " + String(retryCount) + " at " + String(labelToString(currentLabel)));
        approved = validateScanAccuracy();

        if (approved) {
            Serial.println("[VALIDATE] Scan completed successfully at: " + String(labelToString(currentLabel)));
            validatedLabels[currentLabel] = true;
            return;
        }

        Serial.println("[VALIDATE] Accuracy insufficient, retrying... (" + String(retryCount) + "/" + String(MAX_RETRIES) + ")");
        retryCount++;
    }

    Serial.println("[VALIDATE] Failed after max retries at: " + String(labelToString(currentLabel)));
    validatedLabels[currentLabel] = false;
}

void printFinalValidationSummary() {
    Serial.println();
    Serial.println("=============== Validation Summary ===============");
    for (int i = 0; i < NUMBER_OF_LABELS; ++i) {
        String result = validatedLabels[i] ? "VALIDATED" : "NOT VALIDATED";
        Serial.println("[VALIDATE] Label " + String(i) + " - " + String(labelToString((Label) i)) + ": " + result);
    }
    Serial.println("==================================================");
    Serial.println();
}

bool validateScanAccuracy() {
    int matchesRSSI = -1;
    int matchesTOF = -1;

    Serial.println("[VALIDATE] Running predictions...");

    switch (currentSystemState) {
        case STATIC_RSSI:
            Serial.println("[VALIDATE] Mode: STATIC_RSSI");
            matchesRSSI = computeRSSIPredictionMatches();
            break;
        case STATIC_RSSI_TOF:
            Serial.println("[VALIDATE] Mode: STATIC_RSSI_TOF");
            matchesRSSI = computeRSSIPredictionMatches();
            //matchesTOF  = computeTOFPredictionMatches();
            break;
        case STATIC_DYNAMIC_RSSI:
            Serial.println("[VALIDATE] Mode: STATIC_DYNAMIC_RSSI");
            matchesRSSI = computeRSSIPredictionMatches();
            break;
        case STATIC_DYNAMIC_RSSI_TOF:
            Serial.println("[VALIDATE] Mode: STATIC_DYNAMIC_RSSI_TOF");
            matchesRSSI = computeRSSIPredictionMatches();
            //matchesTOF  = computeTOFPredictionMatches();
            break;
        default:
            Serial.println("[VALIDATE] Unknown system state.");
            return false;
    }

    int totalMatches = matchesRSSI + matchesTOF;
    int totalAttempts = (matchesTOF > -1) ? 2 * NUM_OF_VALIDATION_SCANS : NUM_OF_VALIDATION_SCANS;
    accuracy = (100 * totalMatches) / totalAttempts;

    Serial.println("[VALIDATE] Accuracy = " + String(accuracy) + "% (" + String(totalMatches) + "/" + String(totalAttempts) + " correct)");

    bool valid = (accuracy >= VALIDATION_PASS_THRESHOLD);
    return valid && promptUserAccuracyApprove();
}

bool isBackupDataSetRelevant(void) {
    Serial.println("[VALIDATE] Starting backup dataset relevance validation...");

    if ((currentSystemState == STATIC_RSSI_TOF || currentSystemState == STATIC_DYNAMIC_RSSI_TOF)) {
        Serial.println("[VALIDATE] TOF dataset too small for combined state.");
        return false;
    }

    if (rssiDataSet.size() < MIN_VALID_DATA_SET_SIZE) {
        Serial.println("[VALIDATE] RSSI dataset too small.");
        return false;
    }

    unsigned int numOfLabelInRSSI[NUMBER_OF_LABELS] = {0};
    unsigned int numOfLabelInTOF[NUMBER_OF_LABELS] = {0};

    bool isDataSetValid = false;

    for (const RSSIData &data : rssiDataSet) {
        numOfLabelInRSSI[data.label]++;
    }

    for (int i = 0; i < NUMBER_OF_LABELS; ++i) {
        Serial.println("[VALIDATE] Label " + String(i) + " - " + labelToString((Label)i));
        Serial.println("  [VALIDATE] RSSI samples: " + String(numOfLabelInRSSI[i]));
        Serial.println("  [VALIDATE] TOF samples : " + String(numOfLabelInTOF[i]));

        currentLabel = (Label)i;
        if (numOfLabelInRSSI[i] >= MIN_DATA_PER_LABEL_SIZE && numOfLabelInTOF[i] >= 0.5 * MIN_DATA_PER_LABEL_SIZE) {
            currentLabel = (Label)i;
            Serial.println("[VALIDATE] --------------------------------------------");
            Serial.println("[VALIDATE] Please go to label: " + String(labelToString(currentLabel)) + "Press Enter to start validation...");
            while (!Serial.available()) delay(50);
            Serial.read();  // consume newline
        
            Serial.println("[VALIDATE] Checking if backup data is sufficient...");
            bool valid = validateScanAccuracy();
            
            if(!valid) {
                Serial.println("[VALIDATE] Backup dataset not sufficient for: " + String(labelToString((Label)i)));
                continue;
            }

            reuseFromSD[i] = true;
            isDataSetValid = true;
            Serial.println("[VALIDATE] Valid scan data — will reuse backup for: " + String(labelToString((Label)i)));
        }
        else {
            Serial.println("[VALIDATE] Backup dataset not sufficient for: " + String(labelToString((Label)i)));
        }
    }

    if (isDataSetValid) {
        Serial.println("[VALIDATE] At least one label has valid backup data.");
    } else {
        Serial.println("[VALIDATE] No label has sufficient backup data.");
    }

    return isDataSetValid;
}
