
#include "predictionSessionHandler.h"
#include "core/dataManaging/data.h"

void handlePredictionSession() {
    
    loadDataset();
    
    if(!isDataLoadedForPrediction()) {
        LOG_ERROR("MAIN", "Data not available for prediction ...");
        shouldAbort = true;
        return;
    }
    LOG_INFO("SESSION", ">>> Starting PREDICTION_SESSION...");
    runPredictionPhase();
}
