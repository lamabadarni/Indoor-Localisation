/**
 * @file verifier.cpp
 * @brief Implementation of scanning functions for verifying RSSI anchor coverage.
 */

#include <WiFi.h>
#include "utillities.h"
#include "userUI.h"
#include "verifier.h"

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
        Label label = promptLocationSelection();
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
