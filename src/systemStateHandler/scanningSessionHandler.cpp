
#include "scanningSessionHandler.h"
#include "core/ui/userUI.h"
#include "core/scanning/scanningPhase.h"
#include "core/utils/logger.h"

void handleScanningSession() {
    if (!SystemSetup::enableBackup) {
        LOG_ERROR("SETUP", "Scanning mode requires enabled backup loading.");
        LOG_INFO("SETUP", "Please reconfigure system setup.");
        reconfigure = true;
    }

    
}
