/**
 * @file verifier.cpp
 * @brief Implementation of scanning functions for verifying RSSI and TOF anchor coverage.
 */

#include <WiFi.h>
#include "utillities.h"
#include "userUI.h"
#include "verifier.h"
#include "tofScanner.h"

// ================= RSSI =================

RSSICoverageResult scanRSSICoverage(Label label) {
    RSSICoverageResult result = {0};
    result.label = label;

    WiFi.disconnect(true);
    delay(100);
    int n = WiFi.scanNetworks(false, true);  // Passive scan

    int rssiSum = 0;

    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
        result.anchorVisibility[i] = false;
        result.anchorRSSI[i] = RSSI_DEFAULT_VALUE;

        for (int j = 0; j < n; ++j) {
            if (WiFi.BSSIDstr(j).equalsIgnoreCase(anchorSSIDs[i])) {
                result.anchorVisibility[i] = true;
                result.anchorRSSI[i] = WiFi.RSSI(j);
                rssiSum += result.anchorRSSI[i];
                result.visibleAnchors++;
                break;
            }
        }
    }

    result.averageRSSI = (result.visibleAnchors > 0)
                         ? (rssiSum / result.visibleAnchors)
                         : RSSI_DEFAULT_VALUE;

    return result;
}

bool verifyRSSIScanCoverage() {
    while (true) {
        Serial.println("Verify RSSI scan coverage");
        Label label = promptLocationLabel();
        RSSICoverageResult result = scanRSSICoverage(label);

        Serial.println("----- RSSI Coverage Report -----");
        Serial.print("Location: "); Serial.println(labelToString(result.label));
        Serial.print("Visible Anchors: "); Serial.println(result.visibleAnchors);
        Serial.print("Average RSSI: "); Serial.println(result.averageRSSI);

        bool approved = promptRSSICoverageUserFeedback();

        if (!approved) {
            if (result.visibleAnchors < MIN_ANCHORS_VISIBLE) {
                Serial.println("Advice: Increase anchor density in this location.");
            }
            if (result.averageRSSI < MIN_AVERAGE_RSSI_DBM) {
                Serial.println("Advice: Move anchors closer or reduce interference.");
            }

            if (promptAbortForImprovement()) {
                Serial.println("Aborting RSSI coverage verification.");
                return false;
            }
        }

        if (!promptVerifyScanCoverageAtAnotherLabel()) {
            break;
        }
    }

    return true;
}

// ================= TOF =================

TOFCoverageResult scanTOFCoverage(Label label) {
    TOFCoverageResult result = {0};
    result.label = label;

    float distances[NUMBER_OF_RESPONDERS] = {0.0f};
    int totalCm = 0;

    currentScanningLabel = label;
    createTOFScanToMakePrediction(distances);

    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        if (distances[i] >= 0.0f) {
            result.responderVisibility[i] = true;
            result.responderDistance[i] = (int)distances[i];
            totalCm += result.responderDistance[i];
            result.visibleResponders++;
        } else {
            result.responderVisibility[i] = false;
            result.responderDistance[i] = INVALID_DISTANCE_CM;
        }
    }

    result.averageDistance = (result.visibleResponders > 0)
                             ? (totalCm / result.visibleResponders)
                             : INVALID_DISTANCE_CM;

    return result;
}

bool verifyTOFScanCoverage() {
    while (true) {
        Serial.println("Verify TOF responder coverage");
        Label label = promptLocationLabel();
        TOFCoverageResult result = scanTOFCoverage(label);

        Serial.println("----- TOF Coverage Report -----");
        Serial.print("Location: "); Serial.println(labelToString(result.label));
        Serial.print("Visible Responders: "); Serial.println(result.visibleResponders);
        Serial.print("Average Distance: "); Serial.println(result.averageDistance);

        for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
            Serial.printf("  Responder %d: %d cm [%s]\n", i,
                result.responderDistance[i],
                result.responderVisibility[i] ? "visible" : "not visible");
        }

        bool approved = promptRSSICoverageUserFeedback(); // reuse prompt

        if (!approved) {
            if (result.visibleResponders < MIN_RESPONDERS_VISIBLE) {
                Serial.println("Advice: Add more responders or reposition to ensure redundancy.");
            }
            if (result.averageDistance > MAX_AVERAGE_TOF_DISTANCE_CM || result.averageDistance == INVALID_DISTANCE_CM) {
                Serial.println("Advice: Reduce distance between scanner and responders.");
            }

            if (promptAbortForImprovement()) {
                Serial.println("Aborting TOF coverage verification.");
                return false;
            }
        }

        if (!promptVerifyScanCoverageAtAnotherLabel()) {
            break;
        }
    }

    return true;
}
