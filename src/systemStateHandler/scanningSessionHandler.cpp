#include "scanningSessionHandler.h"
#include "core/scanning/scanningPhase.h"
#include "core/validation/validationPhase.h"
#include "core/dataManaging/data.h"

void handleScanningSession() {
    LOG_INFO("SESSION", ">>> Starting SCANNING_SESSION...");
    loadDataset();
    runScanningPhase();
    if(SystemSetup::enableValidationPhase) {
        runValidationPhase();
        LOG_INFO("VALIDATE", "Validation phase complete.");
    }
}
