/**
 * @file verifier.cpp
 * @brief Implementation of scanning functions for verifying RSSI and TOF anchor coverage.
 * @author Lama Badarni
 */

#include <WiFi.h>
#include "utillities.h"
#include "userUI.h"
#include "verifier.h"
#include "tofScanner.h"

// ================= RSSI =================

RSSICoverageResult scanRSSICoverage() {
    RSSICoverageResult result = {0};
    result.label = currentLabel;

    WiFi.disconnect(true);
    delay(100);
    int n = WiFi.scanNetworks(false, true);

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
        Serial.println("[VERIFY] Verifying RSSI scan coverage");
        currentLabel = promptLocationLabel();
        RSSICoverageResult result = scanRSSICoverage(currentLabel);

        Serial.println("[VERIFY] ----- RSSI Coverage Report -----");
        Serial.print("[VERIFY] Location: "); Serial.println(labelToString(currentLabel));
        Serial.print("[VERIFY] Visible Anchors: "); Serial.println(result.visibleAnchors);
        Serial.print("[VERIFY] Average RSSI: "); Serial.println(result.averageRSSI);

        bool approved = promptRSSICoverageUserFeedback();

        if (!approved) {
            if (result.visibleAnchors < MIN_ANCHORS_VISIBLE) {
                Serial.println("[VERIFY] Advice: Increase anchor density in this location.");
            }
            if (result.averageRSSI < MIN_AVERAGE_RSSI_DBM) {
                Serial.println("[VERIFY] Advice: Move anchors closer or reduce interference.");
            }

            if (promptAbortForImprovement()) {
                Serial.println("[VERIFY] Aborting RSSI coverage verification.");
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

TOFCoverageResult scanTOFCoverage() {
    TOFCoverageResult result = {0};
    result.label = currentLabel;

    int totalCm = 0;

    createTOFScanToMakePrediction();

    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        if (accumulatedTOFs[i] >= 0) {
            result.responderVisibility[i] = true;
            result.responderDistance[i] = static_cast<int>(accumulatedTOFs[i]);
            totalCm += result.responderDistance[i];
            result.visibleResponders++;
        } else {
            result.responderVisibility[i] = false;
            result.responderDistance[i] = TOF_DEFAULT_DISTANCE_CM;
        }
    }

    result.averageDistance = (result.visibleResponders > 0)
                             ? (totalCm / result.visibleResponders)
                             : TOF_DEFAULT_DISTANCE_CM;

    return result;
}

bool verifyTOFScanCoverage() {
    while (true) {
        Serial.println("[VERIFY] Verifying TOF responder coverage");
        currentLabel = promptLocationLabel();
        TOFCoverageResult result = scanTOFCoverage();

        Serial.println("[VERIFY] ----- TOF Coverage Report -----");
        Serial.print("[VERIFY] Location: "); Serial.println(labelToString(currentLabel));
        Serial.print("[VERIFY] Visible Responders: "); Serial.println(result.visibleResponders);
        Serial.print("[VERIFY] Average Distance: "); Serial.println(result.averageDistance);

        for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
            Serial.println("[VERIFY]   Responder " + String(i) + ": " + String(result.responderDistance[i]) +
                           " cm [" + (result.responderVisibility[i] ? "visible" : "not visible") + "]");
        }

        bool approved = promptRSSICoverageUserFeedback();

        if (!approved) {
            if (result.visibleResponders < TOF_MIN_RESPONDERS_VISIBLE) {
                Serial.println("[VERIFY] Advice: Add more responders or reposition to ensure redundancy.");
            }
            if (result.averageDistance > TOF_MAX_AVERAGE_DISTANCE_CM ||
                result.averageDistance == TOF_DEFAULT_DISTANCE_CM) {
                Serial.println("[VERIFY] Advice: Reduce distance between scanner and responders.");
            }

            if (promptAbortForImprovement()) {
                Serial.println("[VERIFY] Aborting TOF coverage verification.");
                return false;
            }
        }

        if (!promptVerifyScanCoverageAtAnotherLabel()) {
            break;
        }
    }

    return true;
}