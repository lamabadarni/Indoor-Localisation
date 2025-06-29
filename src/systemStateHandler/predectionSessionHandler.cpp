
#include "src/systemStateHandler/predectionSessionHandler.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "core/prediction/predictionPhase.h"
#include "core/dataManaging/data.h"

void handlePredictionSession() {

    if (!SystemSetup::enableRestore) {
        LOG_ERROR("SETUP", "Prediction mode requires enabled backup loading.");
        LOG_INFO("SETUP", "Please reconfigure system setup.");
        return;
    }

    if (!initDataBackup()) {
        LOG_ERROR("SETUP", "Failed to initialize internal flash.");
        LOG_INFO("SETUP", "Please reconfigure system setup.");
        return;
    }

    runPredictionPhase();
}
