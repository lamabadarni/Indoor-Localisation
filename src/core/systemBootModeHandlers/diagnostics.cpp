#include "core/systemBootModeHandlers/diagnostics.h"
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
    LOG_INFO("COVERAGE", "or move anchors while scanning or running predictions.");
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
        promptUserLocationLabel();
        delay_ms(USER_PROMPTION_DELAY);
        switch (SystemSetup::currentSystemBootMode) {
            case MODE_TOF_DIAGNOSTIC:
            LOG_INFO("VERIFY", "Verifying TOF scan coverage");
            performTOFScanCoverage();
            break;
            case MODE_RSSI_DIAGNOSTIC:
            LOG_INFO("VERIFY", "Verifying RSSI scan coverage");
            performRSSIScanCoverage();
            case MODE_RESTORE_BACKUP_DATA_TEST:
            case MODE_COLLECT_TOF_RESPONDERS_MAC:
            case MODE_INITIATOR_RESPONDER_TEST:
            return false;
            case SYSTEM_BOOT_MODES_NUM:
            //TODO: should have assert or something
            return false;
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
                        ? (sum / result.seen)
                        : RSSI_DEFAULT_VALUE;

    return result;
}

void performRSSIScanCoverage() {
    Coverage result = scanRSSIForCoverage();
    LOG_INFO("VERIFY", "----- RSSI Coverage Report -----");
    LOG_INFO("VERIFY", "Location: %s", labels[currentLabel]);
    LOG_INFO("VERIFY", "Visible Anchors: %d", result.seen);
    LOG_INFO("VERIFY", "Average RSSI: %d ", result.average);

    if (result.seen < MIN_ANCHORS_VISIBLE) {
        LOG_WARN("VERIFY", "Advice: Increase anchor density in this location.");
    }
    if (result.average < MIN_AVERAGE_RSSI) {
        LOG_WARN("VERIFY", "Advice: Move anchors closer or reduce interference.");
    }
}

// ================= TOF =================

Coverage scanTOFForCoverage() {

    int totalCm = 0;
    Coverage result;

    // Lama: check what this call should be replaced by
    performTOFScan();

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

void performTOFScanCoverage() {
    Coverage result = scanTOFForCoverage();

    LOG_INFO("VERIFY", "----- TOF Coverage Report -----");
    LOG_INFO("VERIFY", "Location: %s", labels[currentLabel]);
    LOG_INFO("VERIFY", "Visible Responders: %d", result.seen);
    LOG_INFO("VERIFY", "Average TOF Distances: %d ", result.average);

    delay_ms(USER_PROMPTION_DELAY);
    
    if (result.seen < MIN_ANCHORS_VISIBLE) {
        LOG_WARN("VERIFY", "Advice: Increase responders density in this location.");
    }

    if (result.average < MIN_AVERAGE_RSSI) {
        LOG_WARN("VERIFY", "Advice: Move responders closer or reduce interference.");
    }
    
}
        
