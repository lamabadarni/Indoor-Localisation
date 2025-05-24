#include "utillities.h"
#include "userUI.h"
#include "scanning.h"
#include "predictionPhase.h"
#include "validationPhase.h"
#include "verifier.h"
#include "sdCardBackup.h"


void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=============== System Boot ===============");
    
    // Prompt mode and enablements
    currentSystemMode = promptSystemMode();
    setupEnablementsFromUser();

    Serial.println("=============== System Ready ===============");
}

void loop() {
    
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
    }

    if (!loadLocationDataset()) {
        Serial.println("[SD] FATAL: Failed to load dataset from SD.");
        while (true);
    }

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
