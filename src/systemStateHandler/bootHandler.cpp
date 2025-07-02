
#include "bootHandler.h"
#include "core/utils/logger.h"
#include "core/utils/utilities.h"
#include "core/utils/platform.h"
#include "core/systemBootModeHandlers/diagnostics.h"
#include "core/systemBootModeHandlers/responderMACsCollector.h"

void handleSystemBoot() {
    switch (SystemSetup::currentSystemBootMode) {
        case MODE_TOF_DIAGNOSTIC:
            performTOFScanCoverage();
            break;
        case MODE_RSSI_DIAGNOSTIC:
            performRSSIScanCoverage();
            break;
        case MODE_COLLECT_TOF_RESPONDERS_MAC:
            collectResponderMACs();
            break;
        case MODE_INITIATOR_RESPONDER_TEST: 
            //LAMA: after adding the inititor logic add test here
            break;
        case MODE_RESTORE_BACKUP_DATA_TEST:
            //LAMA: after adding the data add the test here
            break;
        default:
            LOG_WARN("BOOT", "Unsupported boot sub-mode.");
            break;
    }
}

