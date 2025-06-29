
#include "scanningSessionHandler.h"
#include "core/scanning/scanningPhase.h"
#include "core/validation/validationPhase.h"

void handleScanningSession() {
    if (!SystemSetup::enableBackup) {
        LOG_ERROR("SETUP", "Scanning mode requires enabled backup loading.");
        LOG_INFO("SETUP", "Please reconfigure system setup.");
        return;
    }

    runScanningPhase();

    if(SystemSetup::enableValidationPhase) {
        runValidationPhase();
    }

    
}
