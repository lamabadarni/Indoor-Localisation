
#include "bootHandler.h"
#include "core/utils/logger.h"
#include "core/utils/utilities.h"
#include "core/utils/platform.h"
#include "core/systemBootModeHandlers/diagnostics.h"
#include "core/systemBootModeHandlers/responderHandler.h"

void handleSystemBoot() {
    LOG_INFO("SESSION", ">>> Starting SYSTEM_BOOT...");
    switch (SystemSetup::currentSystemBootMode) {
        case MODE_TOF_DIAGNOSTIC:
        case MODE_ANCHORS_RSSI_DIAGNOSTIC:
        case MODE_APS_RSSI_DIAGNOSTIC :
            interactiveScanCoverage();
            break;
        case MODE_INIT_RESPONDER: 
            initFTMResponder();
            break;
        default:
            LOG_ERROR("BOOT", "Unsupported boot sub-mode.");
            break;
    }
}

