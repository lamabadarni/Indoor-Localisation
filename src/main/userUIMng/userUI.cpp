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
    Serial.println("\nUserUI: Select System Mode:");
    for (int i = 0; i < MODE_COUNT; ++i) {
        Serial.println("  " + String(i) + ". " + String(modeToString((SystemMode)i)));
    }

    int selection = -1;
    while (selection < 0 || selection >= MODE_COUNT) {
        Serial.print("Enter mode number: ");
        while (Serial.available() == 0);
        selection = Serial.parseInt();
    }

    currentSystemMode = static_cast<SystemMode>(selection);
    Serial.println("UserUI: Selected mode is " + String(modeToString(currentSystemMode)));
    return currentSystemMode;
}

SystemState promptSystemState() {
    Serial.println("UserUI: Select System State:");
    for (int i = 0; i <= OFFLINE; ++i) {
        Serial.println("  " + String(i) + " - " + String(systemStateToString(i)));
    }

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            int selected = c - '0';
            if (selected >= 0 && selected <= OFFLINE) {
                Serial.println("UserUI: Selected system state is " + String(systemStateToString(selected)));
                return static_cast<SystemState>(selected);
            }
        }
    }
}


void setupEnablementsFromUser() {
    Serial.println("\nUserUI: === Configure Enablements ===");

    currentSystemState = promptSystemState();
    
    struct {
        const char* prompt;
        bool* flag;
        const char* name;
    } options[] = {
        { "Enable SD Card Backup? (1=Yes, 0=No)", &Enablements::enable_SD_Card_backup, "SD Card Backup" },
        { "Enable Training Model on Host? (1=Yes, 0=No)", &Enablements::enable_training_model_on_host_machine, "Training on Host" },
        { "Run Validation Phase? (1=Yes, 0=No)", &Enablements::run_validation_phase, "Validation Phase" },
        { "Verify TOF Responder Mapping? (1=Yes, 0=No)", &Enablements::verify_responder_mac_mapping, "TOF Responder Mapping" },
        { "Verify RSSI Anchor Mapping? (1=Yes, 0=No)", &Enablements::verify_rssi_anchor_mapping, "RSSI Anchor Mapping" }
    };

    for (auto& opt : options) {
        Serial.println(opt.prompt);
        while (!Serial.available());
        *opt.flag = Serial.read() == '1';
        Serial.println("UserUI: " + String(opt.name) + " = " + String(*opt.flag));
    }
}
  }
}



// ====================== Location Selection ======================

Label promptLocationLabel() {
    Serial.println("UserUI: Select Label by Index:");
    for (int i = 0; i < NUMBER_OF_LABELS; ++i) {
        Serial.println("  " + String(i) + " - " + String(labelToString(i)));
    }

    int label = -1;
    while (label < 0 || label >= NUMBER_OF_LABELS) {
        while (Serial.available() == 0);
        label = Serial.parseInt();
    }

    Serial.println("UserUI: selected label is " + String(labelToString(label)));
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
            if (c == '0') {
                Serial.println("UserUI: user rejected scan accuracy.");
                return false;
            }
            if (c == '1') {
                Serial.println("UserUI: user approved scan accuracy.");
                return true;
            }
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

//not used currently
int promptRetryValidationWithSingleMethod() {
    Serial.println("Combined validation not sufficient.");
    Serial.println("Would you like to retry validation using only one method?");
    Serial.println("1 - Retry with RSSI only");
    Serial.println("2 - Retry with TOF only");
    Serial.println("0 - Abort");

    while (Serial.available() == 0);
    int input = Serial.parseInt();

    if (input == 1 || input == 2 || input == 0)
        return input;
    
    return 0; // Default to abort if input is invalid
}


int promptUserPreferredPrediction() {
    Serial.println("UserUI: RSSI and TOF predictions differ.");
    Serial.println("Which prediction do you trust?");
    Serial.println("1 - Trust RSSI");
    Serial.println("2 - Trust TOF");

    int input = -1;
    while (input < 0 || input > 1) {
        while (Serial.available() == 0);
        input = Serial.parseInt();
    }

    Serial.println("UserUI: user selected preference = " + String(input));
    return input;
}


