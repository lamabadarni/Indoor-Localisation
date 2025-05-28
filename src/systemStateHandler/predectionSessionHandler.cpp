
#include "src/systemStateHandler/predectionSessionHandler.h"
#include "../core/utils/utilities.h"
#include "../core/ui/logger.h"
#include "../core/prediction/predictionPhase.h"
#include "../core/dataManaging/internalFlashIO.h"

void handlePredictionSession() {

    if (!SystemSetup::enableRestore) {
        LOG_ERROR("SETUP", "Prediction mode requires enabled backup loading.");
        LOG_INFO("SETUP", "Please reconfigure system setup.");
        reconfigure = true;
        return;
    }

    if (!initInternalFlash()) {
        LOG_ERROR("SETUP", "Failed to initialize internal flash.");
        LOG_INFO("SETUP", "Please reconfigure system setup.");
        reconfigure = true;
        return;
    }

    runPredictionPhase();
}
