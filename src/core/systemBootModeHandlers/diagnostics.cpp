/**
 * @file diagnostics.cpp
 * @brief Implements diagnostic scans for RSSI (static/dynamic) and ToF.
 */

#include "diagnostics.h"
#include "core/scanning/staticRSSIScanner.h"
#include "core/scanning/dynamicRSSIScanner.h"
#include "core/scanning/tofScanner.h"
#include "core/ui/userUI.h"

// ================= STATIC RSSI =================

Coverage scanStaticForCoverage() {
    createSingleStaticRSSIScan();

    int anchorsSeen = 0;
    double sum = 0;
    for (int i = 0; i < NUMBER_OF_ANCHORS; ++i) {
        if (accumulatedStaticRSSIs[i] > MIN_AVERAGE_RSSI) {
            anchorsSeen++;
            sum += accumulatedStaticRSSIs[i];
        }
    }

    double avg = anchorsSeen > 0 ? sum / anchorsSeen : 0;
    LOG_INFO("COVERAGE", "[STATIC] Anchors seen: %d / %d, Avg RSSI: %.2f", anchorsSeen, NUMBER_OF_ANCHORS, avg);

    if (anchorsSeen == 0) return COVERAGE_NONE;
    if (anchorsSeen < NUMBER_OF_ANCHORS / 2) return COVERAGE_WEAK;
    return COVERAGE_GOOD;
}

void performRSSIScanCoverage() {
    LOG_INFO("COVERAGE", "Performing STATIC RSSI diagnostic scan...");
    Coverage cov = scanStaticForCoverage();
    LOG_INFO("COVERAGE", "Static RSSI Coverage: %s", coverages[cov].c_str());
}

// ================= DYNAMIC RSSI =================

Coverage scanDynamicForCoverage() {
    createSingleDynamicRSSIScan();
    if (shouldAbort) return COVERAGE_NONE;

    int count = 0;
    double sum = 0;
    for (const auto& r : accumulatedDynamicRSSIs) {
        sum += r;
        count++;
    }

    double avg = count > 0 ? sum / count : 0;
    LOG_INFO("COVERAGE", "[DYNAMIC] MACs seen: %d, Avg RSSI: %.2f", count, avg);

    if (count == 0) return COVERAGE_NONE;
    if (count < 2) return COVERAGE_WEAK;
    return COVERAGE_GOOD;
}

void performDynamicScanCoverage() {
    LOG_INFO("COVERAGE", "Performing DYNAMIC RSSI diagnostic scan...");
    Coverage cov = scanDynamicForCoverage();
    LOG_INFO("COVERAGE", "Dynamic RSSI Coverage: %s", coverages[cov].c_str());
}

// ================= TOF =================

Coverage scanTOFForCoverage() {
    createSingleTOFScan();

    int respondersSeen = 0;
    double sum = 0;
    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        if (accumulatedTOFs[i] != TOF_DEFAULT_DISTANCE_CM) {
            respondersSeen++;
            sum += accumulatedTOFs[i];
        }
    }

    double avg = respondersSeen > 0 ? sum / respondersSeen : 0;
    LOG_INFO("COVERAGE", "[TOF] Responders seen: %d / %d, Avg distance: %.2f cm", respondersSeen, NUMBER_OF_RESPONDERS, avg);

    if (respondersSeen == 0) return COVERAGE_NONE;
    if (respondersSeen < NUMBER_OF_RESPONDERS / 2) return COVERAGE_WEAK;
    return COVERAGE_GOOD;
}

void performTOFScanCoverage() {
    LOG_INFO("COVERAGE", "Performing TOF diagnostic scan...");
    sscanf(responderMacStr.c_str(), "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX",
               &responderMacs[0][0], &responderMacs[0][1], &responderMacs[0][2],
               &responderMacs[0][3], &responderMacs[0][4], &responderMacs[0][5]);
    Coverage cov = scanTOFForCoverage();
    LOG_INFO("COVERAGE", "TOF Coverage: %s", coverages[cov].c_str());
}

// ================== INTERACTIVE MODE ==================

void interactiveScanCoverage() {
    while (!shouldAbort) {
        
        switch (SystemSetup::currentSystemBootMode) {
            case MODE_ANCHORS_RSSI_DIAGNOSTIC:
                performRSSIScanCoverage();
                break;
            case MODE_APS_RSSI_DIAGNOSTIC:
                performDynamicScanCoverage();
                break;
            case MODE_TOF_DIAGNOSTIC:
                performTOFScanCoverage();
                break;
            default:
                return;
        }

        if (promptUserAbortToImproveEnvironment()) {
            break;
        }
    }
}
