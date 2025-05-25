/**
 * @file verefier.cpp
 * @brief Implementation of scanning functions for verifying RSSI and TOF anchor coverage.
 */

#include <userUI.h>
#include <verefier.h>
#include <tofScanner.h>
#include "utils/logger.h"

// ================= RSSI =================

RSSICoverageResult scanRSSICoverage() {
    RSSICoverageResult result = {(Label)0};
    result.label = currentLabel;

    WiFi.disconnect(true);
    delay_ms(100);
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
        LOG_INFO("VERIFY", "Verifying RSSI scan coverage");
        currentLabel = promptLocationLabel();
        RSSICoverageResult result = scanRSSICoverage();

        LOG_INFO("VERIFY", "----- RSSI Coverage Report -----");
        LOG_INFO("VERIFY", "Location: %s", labelToString(currentLabel));
        LOG_INFO("VERIFY", "Visible Anchors: %d", result.visibleAnchors);
        LOG_INFO("VERIFY", "Average RSSI: %d dBm", result.averageRSSI);

        bool approved = promptRSSICoverageUserFeedback();

        if (!approved) {
            if (result.visibleAnchors < MIN_ANCHORS_VISIBLE) {
                LOG_WARN("VERIFY", "Advice: Increase anchor density in this location.");
            }
            if (result.averageRSSI < MIN_AVERAGE_RSSI_DBM) {
                LOG_WARN("VERIFY", "Advice: Move anchors closer or reduce interference.");
            }

            if (promptAbortForImprovement()) {
                LOG_WARN("VERIFY", "Aborting RSSI coverage verification.");
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
    TOFCoverageResult result = {(Label)0};
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
        LOG_INFO("VERIFY", "Verifying TOF responder coverage");
        currentLabel = promptLocationLabel();
        TOFCoverageResult result = scanTOFCoverage();

        LOG_INFO("VERIFY", "----- TOF Coverage Report -----");
        LOG_INFO("VERIFY", "Location: %s", labelToString(currentLabel));
        LOG_INFO("VERIFY", "Visible Responders: %d", result.visibleResponders);
        LOG_INFO("VERIFY", "Average Distance: %d cm", result.averageDistance);

        for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
            const char* vis = result.responderVisibility[i] ? "visible" : "not visible";
            LOG_INFO("VERIFY", "  Responder %d: %d cm [%s]", i, result.responderDistance[i], vis);
        }

        bool approved = promptRSSICoverageUserFeedback();

        if (!approved) {
            if (result.visibleResponders < TOF_MIN_RESPONDERS_VISIBLE) {
                LOG_WARN("VERIFY", "Advice: Add more responders or reposition to ensure redundancy.");
            }
            if (result.averageDistance > TOF_MAX_AVERAGE_DISTANCE_CM ||
                result.averageDistance == TOF_DEFAULT_DISTANCE_CM) {
                LOG_WARN("VERIFY", "Advice: Reduce distance between scanner and responders.");
            }

            if (promptAbortForImprovement()) {
                LOG_WARN("VERIFY", "Aborting TOF coverage verification.");
                return false;
            }
        }

        if (!promptVerifyScanCoverageAtAnotherLabel()) {
            break;
        }
    }

    return true;
}
