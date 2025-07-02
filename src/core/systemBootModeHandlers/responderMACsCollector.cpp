/**
 * @file responderMACsCollector.cpp
 * @brief Collects MAC addresses of visible FTM responders and allows user to store them.
 */

#include "responderMACsCollector.h"
#include "core/diagnostics/diagnostics.h"
#include "core/utils/logger.h"
#include "core/ui/userUI.h"

bool isMACUnset(const uint8_t* mac) {
    for (int i = 0; i < MAC_ADDRESS_SIZE; ++i) {
        if (mac[i] != 0) return false;
    }
    return true;
}

void clearResponderMacs() {
    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        memset(responderMacs[i], 0, MAC_ADDRESS_SIZE);
    }
}

String formatMac(const uint8_t* mac) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

void collectResponderMACs() {
    // Check if already configured
    bool alreadyConfigured = true;
    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        if (isMACUnset(responderMacs[i])) {
            alreadyConfigured = false;
            break;
        }
    }

    if (alreadyConfigured) {
        LOG_INFO("RESPONDERS", "[USER] > All responder MACs are already configured.");
        LOG_INFO("RESPONDERS", "[USER] > Would you like to scan again anyway? (y/n): ");
        char res = readCharFromUser();
        if (res != 'y' && res != 'Y') {
            return;
        }
    }

    // Perform scan
    Coverage coverage = scanTOFForCoverage();
    if (coverage == COVERAGE_NONE) {
        LOG_ERROR("RESPONDERS", "No FTM responders found. Aborting.");
        return;
    }

    LOG_INFO("RESPONDERS", "Detected the following FTM responders:");
    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        LOG_INFO("RESPONDERS", "  [%d] %s", i + 1, formatMac(responderMacs[i]).c_str());
    }

    LOG_INFO("RESPONDERS", "Would you like to save these MACs? (y/n): ");
    char confirm = readCharFromUser();
    if (confirm != 'y' && confirm != 'Y') {
        clearResponderMacs();
        LOG_INFO("RESPONDERS", "MAC list cleared.");
    } else {
        LOG_INFO("RESPONDERS", "MACs saved to global table.");
    }
}