/**
 * @file verefier.cpp
 * @brief Implementation of scanning functions for verifying RSSI and TOF anchor coverage.
 */

 #include "../utils/utilities.h"
#include "../ui/userUI.h"
#include "../ui/verefier.h"
#include "../scanning/rssiScanner.h"
#include "../scanning/tofScanner.h"
#include "../utils/platform.h"

typedef struct {
    int    seen    = 0;
    double average = 0;
} Coverage;

static printStartLog() {
    LOG_INFO("COVERAGE", "------------------------------------------------------------");
    LOG_INFO("COVERAGE", "Interactive RSSI Coverage Diagnostic");
    LOG_INFO("COVERAGE", "This diagnostic helps evaluate the signal strength and balance");
    LOG_INFO("COVERAGE", "from all anchors currently visible at your location.");
    LOG_INFO("COVERAGE", "------------------------------------------------------------");
    
    LOG_INFO("COVERAGE", "IMPORTANT:");
    LOG_INFO("COVERAGE", "DO NOT change the environment (e.g., walk away, close doors,");
    LOG_INFO("COVERAGE", "or move anchors) while scanning or running predictions.");
    LOG_INFO("COVERAGE", "Changing the environment in the middle of a scan will cause");
    LOG_INFO("COVERAGE", "inaccurate results and misleading coverage analysis.");
    
    LOG_INFO("COVERAGE", "Run this diagnostic only when the environment is STABLE and IDLE.");
    LOG_INFO("COVERAGE", "------------------------------------------------------------");

}

static bool startCalled = false;

bool interactiveScanCoverage() {
    if(!startCalled) {
        printStartLog();
        startCalled = true;
    }
    while (true) {
        promptLocationLabel();
        switch (currentSystemMode) {
            case MODE_TOF_DIAGNOSTIC:
            LOG_INFO("VERIFY", "Verifying TOF scan coverage");
            performTOFScanCoverage();
            break;
            case MODE_RSSI_DIAGNOSTIC:
            LOG_INFO("VERIFY", "Verifying RSSI scan coverage");
            performRSSIScanCoverage();
        }

        if (promptAbortForImprovementAfterCoverage()) {
            LOG_WARN("VERIFY", "Aborting coverage diagnostic for improvement.");
            shouldAbort = true;
            return false;
        }

        if (!promptVerifyScanCoverageAtAnotherLabel()) {
            break;
        }

        promptAbotOrContinue();
    }

    return true;
}

// ================= RSSI =================

void performRSSIScanCoverage() {
    RSSICoverageResult result = scanRSSICoverage();
    LOG_INFO("VERIFY", "----- RSSI Coverage Report -----");
    LOG_INFO("VERIFY", "Location: %s", labelToString(currentLabel));
    LOG_INFO("VERIFY", "Visible Anchors: %d", result.visibleAnchors);
    LOG_INFO("VERIFY", "Average RSSI: %d ", result.averageRSSI);

    bool approved = promptRSSICoverageUserFeedback();

    if (!approved) {
        if (result.visibleAnchors < MIN_ANCHORS_VISIBLE) {
            LOG_WARN("VERIFY", "Advice: Increase anchor density in this location.");
        }
        if (result.averageRSSI < MIN_AVERAGE_RSSI_DBM) {
            LOG_WARN("VERIFY", "Advice: Move anchors closer or reduce interference.");
        }
    }
}

 Coverage scanRSSIForCoverage() {

    Coverage result;
    int sum = 0;
    
    RSSIData scanData = createSingleRSSIScan();

    for (int i = 0; i < NUMBER_OF_ANCHORS; i++) {
        if(accumulatedRSSIs[i] != RSSI_DEFAULT_VALUE) {
            result.seen++;
            sum += accumulatedRSSIs[i];
        }
    }

    result.average = (result.senn > 0)
                        ? (rssiSum / result.visibleAnchors)
                        : RSSI_DEFAULT_VALUE;

    return result;
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
        delay_ms(USER_PROMPTION_DELAY);
        promptLocationLabel();
        delay_ms(USER_PROMPTION_DELAY);
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
        delay_ms(USE);
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

        delay_ms(USER_PROMPTION_DELAY);
        if (!promptVerifyScanCoverageAtAnotherLabel()) {
            break;
        }
        delay_ms(USER_PROMPTION_DELAY);
    }

    return true;
}
