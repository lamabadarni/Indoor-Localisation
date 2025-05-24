/**
 * @file userUI.cpp
 * @brief Implementation of user prompts for system configuration, location selection,
 *        validation approval, and diagnostic control.
 */

#include "userUI.h"
#include "utillities.h"
#include "configManager.h"

// ====================== System Setup ======================

SystemMode promptSystemMode() {
    Serial.println("\nSelect System Mode:");
    for (int i = 0; i < MODE_COUNT; ++i) {
        Serial.printf("  %d. %s\n", i, modeToString((SystemMode)i));
    }

    int selection = -1;
    while (selection < 0 || selection >= MODE_COUNT) {
        Serial.print("Enter mode number: ");
        while (Serial.available() == 0);
        selection = Serial.parseInt();
    }

    currentSystemMode = static_cast<SystemMode>(selection);
    Serial.printf("Selected Mode: %s\n\n", modeToString(currentSystemMode));
    return currentSystemMode;
}

SystemState promptSystemState() {
    Serial.println("Select System State:");
    Serial.println("0 - STATIC_RSSI");
    Serial.println("1 - STATIC_RSSI_TOF");
    Serial.println("2 - STATIC_DYNAMIC_RSSI");
    Serial.println("3 - STATIC_DYNAMIC_RSSI_TOF");
    Serial.println("4 - OFFLINE");

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c >= '0' && c <= '4') return static_cast<SystemState>(c - '0');
        }
    }
}

void setupEnablementsFromUser() {
    Serial.println("\n=== Configure Enablements ===");

    Enablements::currentSystemState = promptSystemState();

    Serial.println("Enable SD Card Backup? (1=Yes, 0=No)");
    while (!Serial.available());
    Enablements::enable_SD_Card_backup = Serial.read() == '1';

    Serial.println("Enable Training Model on Host? (1=Yes, 0=No)");
    while (!Serial.available());
    Enablements::enable_training_model_on_host_machine = Serial.read() == '1';

    Serial.println("Run Prediction Phase? (1=Yes, 0=No)");
    while (!Serial.available());
    Enablements::run_prediction_phase = Serial.read() == '1';

    Serial.println("Evaluate TOF Responder Placement? (1=Yes, 0=No)");
    while (!Serial.available());
    Enablements::evaluate_responder_placement = Serial.read() == '1';

    Serial.println("Verify TOF Responder Mapping? (1=Yes, 0=No)");
    while (!Serial.available());
    Enablements::verify_responder_mac_mapping = Serial.read() == '1';

    Serial.println("Verify RSSI Anchor Mapping? (1=Yes, 0=No)");
    while (!Serial.available());
    Enablements::verify_rssi_anchor_mapping = Serial.read() == '1';
}


// ====================== Location Selection ======================

char* promptLocationSelection() {
    Serial.println("Select Location:");
    for (int i = 1; i <= NUMBER_OF_LOCATIONS; ++i) {
        Serial.printf("%d - %d\n", i, i);
    }

    int loc = -1;
    while (loc < 0 || loc >= NUMBER_OF_LOCATIONS) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c >= '0' && c <= '9') loc = c - '0';
            if (c == '1') {
                switch (Serial.peek()) {
                    case '0': Serial.read(); loc = 10; break;
                    case '1': Serial.read(); loc = 11; break;
                    case '2': Serial.read(); loc = 12; break;
                    case '3': Serial.read(); loc = 13; break;
                    case '4': Serial.read(); loc = 14; break;
                    case '5': Serial.read(); loc = 15; break;
                    case '6': Serial.read(); loc = 16; break;
                    case '7': Serial.read(); loc = 17; break;
                }
            }
        }
    }

    return locationToString(loc);
}

Label promptLocationLabel() {
    Serial.println("Select Label by Index:");
    for (int i = 0; i < NUMBER_OF_LABELS; ++i) {
        Serial.printf("  %d - %s\n", i, labelToString(i));
    }

    int label = -1;
    while (label < 0 || label >= NUMBER_OF_LABELS) {
        while (Serial.available() == 0);
        label = Serial.parseInt();
    }

    return static_cast<Label>(label);
}


// ====================== User Decision Prompts ======================

bool promptUserAccuracyApprove() {
    Serial.println("Select option:");
    Serial.println("0 - Accuracy Not Sufficient. Proceed more scans at current label");
    Serial.println("1 - Accuracy Approved.");

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '0') return false;
            if (c == '1') return true;
        }
    }
}

bool promptUserSDCardInitializationApprove() {
    Serial.println("Select option:");
    Serial.println("0 - Retry initiating SD card.");
    Serial.println("1 - Skip initiating SD card.");

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '0') return false;
            if (c == '1') return true;
        }
    }
}

bool promptRSSICoverageUserFeedback() {
    Serial.println("Select option:");
    Serial.println("0 - Bad coverage.");
    Serial.println("1 - Good coverage.");

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '0') return false;
            if (c == '1') return true;
        }
    }
}

bool promptVerifyScanCoverageAtAnotherLabel() {
    Serial.println("Do you want to verify scan coverage at another label? (1 = yes, 0 = no): ");
    while (Serial.available() == 0);
    int input = Serial.parseInt();
    return input == 1;
}

bool promptAbortForImprovement() {
    Serial.println("Do you want to abort and reconfigure before continuing? (1 = yes, 0 = no): ");
    while (Serial.available() == 0);
    int input = Serial.parseInt();
    return input == 1;
}

