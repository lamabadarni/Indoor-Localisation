/**
 * @file verefier.cpp
 * @brief Implementation of scanning functions for verifying RSSI and TOF anchor coverage.
 * @author Lama Badarni
 */

#include <WiFi.h>
#include <userUI.h>
#include <verefier.h>
// ================= RSSI =================

RSSICoverageResult scanRSSICoverage() {
    RSSICoverageResult result = {(Label)0};
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
        RSSICoverageResult result = scanRSSICoverage();

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