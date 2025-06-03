#include "diagnostics.h"
#include "core/scanning/rssiScanner.h"
#include "core/scanning/tofScanner.h"

static void printStartLog() {
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
        delay_ms(USER_PROMPTION_DELAY);
    }
    while (true) {
        // Lama: check new name of this function
        promptLocationLabel();
        delay_ms(USER_PROMPTION_DELAY);
        switch (currentSystemMode) {
            case MODE_TOF_DIAGNOSTIC:
            LOG_INFO("VERIFY", "Verifying TOF scan coverage");
            performTOFScanCoverage();
            break;
            case MODE_RSSI_DIAGNOSTIC:
            LOG_INFO("VERIFY", "Verifying RSSI scan coverage");
            performRSSIScanCoverage();
        }

        if (promptUserAbortToImproveEnvironment()) {
            LOG_WARN("VERIFY", "Aborting coverage diagnostic for improvement.");
            shouldAbort = true;
            return false;
        }

        promptUserAbortOrContinue();
        
        if(shouldAbort) {
            return false;
        }

        delay_ms(USER_PROMPTION_DELAY);
    }

    return true;
}

// ================= RSSI =================

void performRSSIScanCoverage() {
    Coverage result = scanRSSICoverage();
    LOG_INFO("VERIFY", "----- RSSI Coverage Report -----");
    LOG_INFO("VERIFY", "Location: %s", labels[currentLabel]);
    LOG_INFO("VERIFY", "Visible Anchors: %d", result.seen);
    LOG_INFO("VERIFY", "Average RSSI: %d ", result.average);

    if (result.visibleAnchors < MIN_ANCHORS_VISIBLE) {
        LOG_WARN("VERIFY", "Advice: Increase anchor density in this location.");
    }
    if (result.averageRSSI < MIN_AVERAGE_RSSI) {
        LOG_WARN("VERIFY", "Advice: Move anchors closer or reduce interference.");
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

    result.average = (result.seen > 0)
                        ? (rssiSum / result.visibleAnchors)
                        : RSSI_DEFAULT_VALUE;

    return result;
}

// ================= TOF =================

void performTOFScanCoverage() {
    // Lama: update struct name and logs if needed
    RSSICoverageResult result = scanRSSICoverage();

    LOG_INFO("VERIFY", "----- RSSI Coverage Report -----");
    LOG_INFO("VERIFY", "Location: %s", labels[currentLabel]);
    LOG_INFO("VERIFY", "Visible Responders: %d", result.visibleAnchors);
    LOG_INFO("VERIFY", "Average TOF Distances: %d ", result.averageRSSI);F

    delay_ms(USER_PROMPTION_DELAY);
    
    if (result.visibleAnchors < MIN_ANCHORS_VISIBLE) {
        LOG_WARN("VERIFY", "Advice: Increase responders density in this location.");
    }

    if (result.averageRSSI < MIN_AVERAGE_RSSI) {
        LOG_WARN("VERIFY", "Advice: Move responders closer or reduce interference.");
    }
    
}

Coverage scanTOFCoverage() {

    int totalCm = 0;
    Coverage result;

    // Lama: check what this call should be replaced by
    TOFData scanData = createTOFScanToMakePrediction();

    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        if(accumulatedTOFs[i] != TOF_DEFAULT_DISTANCE_CM) {
            result.seen++;
            totalCm += accumulatedTOFs[i];
        }
    }

    result.average = (result.seen > 0)
                             ? (totalCm / result.seen)
                             : TOF_DEFAULT_DISTANCE_CM;

    return result;
}
        
