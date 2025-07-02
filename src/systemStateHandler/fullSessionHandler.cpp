
#include "../core/utils/utilities.h"
#include "../core/utils/platform.h"

#include "fullSessionHandler.h"
#include "../core/ui/userUI.h"
#include "../core/ui/userUIOled.h"
#include "../core/scanning/scanningPhase.h"
#include "../core/validation/validationPhase.h"
#include "../core/prediction/predictionPhase.h"


void handleFullSession() {
    LOG_INFO("SESSION", ">>> Starting FULL_SESSION...");
    runScanningPhase();
    if(SystemSetup::enableValidationPhase) {
        runValidationPhase();
        LOG_INFO("VALIDATE", "Validation phase complete.");
    }
    runPredictionPhase();
    return;
}
