
#include "bootHandler.h"
#include "../core/ui/logger.h"
#include "../core/utils/utilities.h"
#include "../core/utils/platform.h"
#include "../core/ui/verifier.h"

static bool isMACUnset(const uint8_t mac[6]) {
    for (int i = 0; i < 6; ++i) {
        if (mac[i] != 0x00) {
            return false;
        }
    }
    return true;
}

static void clearResponderMacs(uint8_t macs[NUMBER_OF_RESPONDERS][6]) {
    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        memset(macs[i], 0x00, 6);
    }
}

static std::string formatMac(const uint8_t mac[6]) {
    char buffer[18]; // "AA:BB:CC:DD:EE:FF" + null
    snprintf(buffer, sizeof(buffer),
             "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2],
             mac[3], mac[4], mac[5]);
    return std::string(buffer);
}

static void collectResponderMACs() {
    LOG_INFO("MAC", "========== Responder MAC Collection ==========");

    // Check if already filled
    bool allSet = true;
    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        if (isMACUnset(responderMacs[i])) {
            allSet = false;
            break;
        }
    }

    if (allSet) {
        LOG_INFO("MAC", "All responder MACs are already configured.");
        LOG_INFO("MAC", "Would you like to scan again anyway? (y/n)");
        char c = readCharFromUser();
        if (c != 'y' && c != 'Y') return;
    }

    delay_ms(USER_PROMPTION_DELAY);

    int found = verifyTOFScanCoverage();

    if (found == 0) {
        LOG_WARN("MAC", " No FTM-capable responders found.");
        LOG_INFO("MAC", "Please make sure responders are powered and within range.");
        return;
    }

    LOG_INFO("MAC", "Discovered %d responder(s).", found);
    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        if (!isMACUnset(responderMacs[i])) {
            LOG_INFO("MAC", "Responder %d MAC: %s", i + 1, formatMac(responderMacs[i]).c_str());
        }
    }

    LOG_INFO("MAC", "Would you like to save these MACs? (y/n)");
    char confirm = readCharFromUser();
    if (confirm != 'y' && confirm != 'Y') {
        LOG_WARN("MAC", "User chose not to save MAC addresses. Reverting.");
        clearResponderMacs(responderMacs);
    } else {
        LOG_INFO("MAC", "Responder MACs saved.");
    }

    LOG_INFO("MAC", "==============================================");
}


void handleSystemBoot() {
    switch (SystemSetup::currentSystemBootMode) {
        case MODE_TOF_DIAGNOSTIC:
            verifyTOFScanCoverage();
            break;
        case MODE_RSSI_DIAGNOSTIC:
            verifyRSSIScanCoverage();
            break;
        case MODE_COLLECT_TOF_RESPONDERS_MAC:
            collectResponderMACs();
            break;
        case MODE_RESTORE_DATA_TEST:
        
        break;
        default:
            LOG_WARN("BOOT", "Unsupported boot sub-mode.");
            break;
    }
}

