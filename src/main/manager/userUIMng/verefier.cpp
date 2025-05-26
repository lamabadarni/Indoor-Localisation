/**
 * @file verefier.cpp
 * @brief Implementation of scanning functions for verifying RSSI anchor coverage.
 * @author Lama Badarni
 */

#include <WiFi.h>
#include <userUI.h>
#include <verefier.h>
#include <sdCardBackup.h>

static int seen = 0;
// ================= RSSI =================

double scanRSSICoverage() {
    seen = 0; // Reset seen anchors count
    WiFi.disconnect(true);
    delay(100);

    int rssiSum = 0;

    int n = WiFi.scanNetworks(false, true);
    for (int j = 0; j < n; ++j) {
        String ssid = WiFi.SSID(j);
        int rssi = WiFi.RSSI(j);
            for (int k = 0; k < NUMBER_OF_ANCHORS; ++k) {
                if (ssid.equals(anchorSSIDs[k])) {
                    Serial.print("[VERIFY] SEEN SSID: "); Serial.println(ssid);
                    Serial.print("[VERIFY] SEEN RSSI: "); Serial.println(String(rssi));
                    rssiSum += rssi;
                    seen++;
                }
            }
            
    }

    Serial.println("[VERIFY] Scan completed. Anchors seen: " + String(seen));

    return (seen > 0) ? (rssiSum / seen) : RSSI_DEFAULT_VALUE;
}

bool verifyRSSIScanCoverage() {
    while (true) {
        Serial.println("[VERIFY] Verifying RSSI scan coverage");
        currentLabel = promptLocationLabel();
        int result = scanRSSICoverage();

        Serial.println("[VERIFY] ----- RSSI Coverage Report -----");
        Serial.print("[VERIFY] Location: "); Serial.println(labelToString(currentLabel));
        Serial.print("[VERIFY] Visible Anchors: "); Serial.println(seen);
        Serial.print("[VERIFY] Average RSSI: "); Serial.println(result);

        break;
       /* bool approved = promptRSSICoverageUserFeedback();

        if (!approved) {
            if (seen < MIN_ANCHORS_VISIBLE) {
                Serial.println("[VERIFY] Advice: Increase anchor density in this location.");
            }
            if (result < MIN_AVERAGE_RSSI_DBM) {
                Serial.println("[VERIFY] Advice: Move anchors closer or reduce interference.");
            }

            if (promptAbortForImprovement()) {
                Serial.println("[VERIFY] Aborting RSSI coverage verification.");
                return false;
            }
        }
        if (!promptVerifyScanCoverageAtAnotherLabel()) {
            break;
        }*/
    }

    return true;
}