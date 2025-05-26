/**
 * @file userUI.cpp
 * @brief Implementation of user prompts for system configuration, label selection, and approval logic.
 * 
 * This module provides interactive terminal-based UI functions for selecting:
 * - System mode and state
 * - Enablement flags like SD card logging and diagnostics
 * - Label selection for scanning or validation
 * - User approval for scan accuracy, coverage, and prediction choices
 *
 * It ensures structured user feedback at each critical step during scanning and validation phases.
 * 
 * @author Lama Badarni
 */


#include <userUI.h>

// ====================== System Setup ======================

SystemMode promptSystemMode() {
    Serial.println("\n[USER] Select System Mode:");
    for (int i = 0; i < MODE_COUNT; ++i) {
        Serial.println("  " + String(i) + ". " + String(modeToString((SystemMode)i)));
    }

    int selection = -1;
    while (selection < 0 || selection >= MODE_COUNT) {
        Serial.print("[USER] Enter mode number: ");
        while (Serial.available() == 0);
        selection = Serial.parseInt();
    }

    currentSystemMode = static_cast<SystemMode>(selection);
    Serial.println("[USER] Selected mode is " + String(modeToString(currentSystemMode)));
    return currentSystemMode;
}

SystemState promptSystemState() {
    Serial.println("[USER] Select System State:");
    for (int i = 0; i <= OFFLINE; ++i) {
        Serial.println("  " + String(i) + " - " + String(systemStateToString(i)));
    }

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            int selected = c - '0';
            if (selected >= 0 && selected <= OFFLINE) {
                Serial.println("[USER] Selected system state is " + String(systemStateToString(selected)));
                return static_cast<SystemState>(selected);
            }
        }
    }
}

void setupEnablementsFromUser() {
    Serial.println("\n[USER] === Configure Enablements ===");

    currentSystemState = promptSystemState();

    struct {
        const char* prompt;
        bool* flag;
        const char* name;
    } options[] = {
        { "[USER] Enable SD Card Backup? (1=Yes, 0=No)", &Enablements::enable_SD_Card_backup, "SD Card Backup" },
        { "[USER] Enable Training Model on Host? (1=Yes, 0=No)", &Enablements::enable_training_model_on_host_machine, "Training on Host" },
        { "[USER] Run Validation Phase? (1=Yes, 0=No)", &Enablements::run_validation_phase, "Validation Phase" },
        { "[USER] Verify RSSI Anchor Mapping? (1=Yes, 0=No)", &Enablements::verify_rssi_anchor_mapping, "RSSI Anchor Mapping" }
    };

    char c;
    for (auto& opt : options) {
        Serial.println(opt.prompt);
        while (true) {
            if (Serial.available()) {
                 c = Serial.read();
                if (c == '0' || c == '1') {
                    break;
                }
            }
        }
        *opt.flag = c == '1';
        delay(2000);
        Serial.println("[USER] " + String(opt.name) + " = " + String(*opt.flag));
    }
}

// ====================== Location Selection ======================

Label promptLocationLabel() {
    Serial.println("[USER] Select Label by Index:");
    for (int i = 1; i < NUMBER_OF_LABELS; ++i) {
        Serial.println("  " + String(i) + " - " + String(labelToString((Label)i)));
    }

    int label = -1;
    while (label <= 0 || label >= NUMBER_OF_LABELS) {
        while (Serial.available() == 0);
        label = Serial.parseInt();
    }

    Serial.println("[USER] selected label is " + String(labelToString(label)));
    return static_cast<Label>(label);
}

// ====================== User Decision Prompts ======================

bool promptUserAccuracyApprove() {
    Serial.println("[USER] Select option:");
    Serial.println("[USER] 0 - Accuracy Not Sufficient. Proceed more scans at current label");
    Serial.println("[USER] 1 - Accuracy Approved.");

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '0') {
                Serial.println("[USER] user rejected scan accuracy.");
                return false;
            }
            if (c == '1') {
                Serial.println("[USER] user approved scan accuracy.");
                return true;
            }
        }
    }
}

bool promptUserSDCardInitializationApprove() {
    Serial.println("[USER] Select option:");
    Serial.println("[USER] 0 - Retry initiating SD card.");
    Serial.println("[USER] 1 - Skip initiating SD card.");

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '0') return false;
            if (c == '1') return true;
        }
    }
}

bool promptRSSICoverageUserFeedback() {
    Serial.println("[USER] Select option:");
    Serial.println("[USER] 0 - Bad coverage.");
    Serial.println("[USER] 1 - Good coverage.");

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '0') return false;
            if (c == '1') return true;
        }
    }
}

bool promptVerifyScanCoverageAtAnotherLabel() {
    Serial.println("[USER] Do you want to verify scan coverage at another label? (1 = yes, 0 = no): ");
    while (Serial.available() == 0);
    int input = Serial.parseInt();
    return input == 1;
}

bool promptAbortForImprovement() {
    Serial.println("[USER] Do you want to abort and reconfigure before continuing? (1 = yes, 0 = no): ");
    while (Serial.available() == 0);
    int input = Serial.parseInt();
    return input == 1;
}
