#include "utillities.h"
#include "userUI.h"
#include "scanning.h"
#include "predictionPhase.h"
#include "validationPhase.h"
#include "./userUIMng/verefire.h"
#include "sdCardBackup.h"


void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=============== System Boot ===============");
    
    // Prompt mode and enablements
    currentSystemMode = promptSystemMode();
    setupEnablementsFromUser();

    handleSDAndEnablements();
    
    Serial.println("=============== System Ready ===============");
}

void loop() {
    Serial.println("[MAIN] Starting execution based on selected mode...");
    switch(currentSystemMode) {
        case MODE_RSSI_MODEL_DIAGNOSTIC:
        Serial.println("[MAIN] MODE_RSSI_MODEL_DIAGNOSTIC: Running RSSI diagnostics.");
        verifyRSSIScanCoverage();
        break;

        case MODE_TOF_DIAGNOSTIC:
        Serial.println("[MAIN] MODE_TOF_DIAGNOSTIC: Running TOF diagnostics.");
        verifyTOFScanCoverage();
        break;

        case MODE_TRAINING_ONLY:
        Serial.println("[MAIN] MODE_TRAINING_ONLY: Running scanning phase only.");
        runScanningPhase();
        if (Enablements::run_validation_phase) {
            Serial.println("[MAIN] Validation phase is enabled. Starting validation.");
            runValidationPhase();
        }
        else {
            Serial.println("[MAIN] Validation phase is disabled. Skipping.");
        }
        break;

        case MODE_PREDICTION_ONLY:
        Serial.println("[MAIN] MODE_PREDICTION_ONLY: Running prediction phase.");
        runPredictionPhase();
        break;

        case MODE_FULL_SESSION:
        Serial.println("[MAIN] MODE_FULL_SESSION: Running scan, prediction, and optional validation.");
        runScanningPhase();
        if (Enablements::run_validation_phase) {
            Serial.println("[MAIN] Validation phase is enabled. Starting validation.");
            runValidationPhase();
        }
        else {
            Serial.println("[MAIN] Validation phase is disabled. Skipping.");
        }
        runPredictionPhase();
        break;

        default:
        Serial.println("[MAIN] Unknown mode. Restart device.");
        break;
    }
    
    Serial.println("[MAIN] Session complete. Enter 'r' to restart or reset device.");
    while (true);
}

void handleSDAndEnablements() {

  switch (currentSystemMode) {
    case MODE_PREDICTION_ONLY:
      handlePredictionOnlySDLogic();
      break;
    case MODE_FULL_SESSION:
    case MODE_TRAINING_ONLY:
      handleTrainingOrFullSDLogic();
      break;
    default:
      break;  // No SD handling needed for diagnostic modes
  }
}

void handlePredictionOnlySDLogic() {
    int retries = 0;
    bool sdAvailable = false;

    while (!sdAvailable && retries < MAX_RETRIES_TO_INIT_SD_CARD) {
        sdAvailable = initSDCard(csPin);
        if (sdAvailable) break;

        Serial.println("[SD] Required SD card not found.");
        if (!promptUserSDCardInitializationApprove()) {
            Serial.println("[SD] Aborting system - cannot predict without SD card.");
            while (true);
        }
        retries++;
        delay(150);
    }

    if (!loadLocationDataset()) {
        Serial.println("[SD] FATAL: Failed to load dataset from SD.");
        while (true);
    }

    ///check that all location are valid
    if (!isBackupDataSetRelevant()) {
        Serial.println("[SD] FATAL: Backup data is invalid for prediction.");
        while (true);
    }

    Serial.println("[SD] Prediction mode will use existing backup dataset.");
}

void handleTrainingOrFullSDLogic() {
    if (!Enablements::enable_SD_Card_backup) return;

    int retries = 0;
    bool sdAvailable = false;

    while (!sdAvailable && retries < MAX_RETRIES_TO_INIT_SD_CARD) {
        sdAvailable = initSDCard(csPin);
        if (sdAvailable) break;

        Serial.println("[SD] Backup is enabled, but SD card not detected.");
        if (!promptUserSDCardInitializationApprove()) {
            Serial.println("[SD] Proceeding without backup.");
            return;
        }
        retries++;

        delay(150);
    }

    if (!sdAvailable) {
        Serial.println("[SD] SD unavailable after retries. Backup disabled.");
        return;
    }

    String CurrDir = getSDBaseDir();
    if (!SD.exists(CurrDir)) return;

    if (loadLocationDataset()) {
        Serial.println("[SD] FATAL: Failed to load dataset from SD.");
        return;
    }

    if (isBackupDataSetRelevant()) {
        Serial.println("[SD] Found valid backup data.");
        if (promptUserAccuracyApprove()) {
            if (updateCSV()) {
                Serial.println("[SD] Using backup dataset.");
                return;
            } else {
                Serial.println("[SD] updateCSV failed. Proceeding to rescan.");
            }
        } else {
            Serial.println("[SD] User declined reuse.");
        }
    } else {
        Serial.println("[SD] Backup not sufficient for reuse.");
    }

    if (resetStorage()) {
        Serial.println("[SD] Initialized new backup folder.");
        rssiDataSet.clear();
        tofDataSet.clear();
    } else {
        Serial.println("[SD] Failed to create new backup storage.");
    }
}
