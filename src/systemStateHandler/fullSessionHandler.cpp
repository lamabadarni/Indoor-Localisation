
#include "../core/utils/utilities.h"
#include "../core/utils/platform.h"

#include "fullSessionHandler.h"
#include "../core/ui/userUI.h"
#include "../core/ui/userUIOled.h"
#include "../core/scanning/scanningPhase.h"
#include "../core/validation/validationPhase.h"
#include "../core/prediction/predictionPhase.h"


void handleFullSession() {
    runScanningPhase();
    if(SystemSetup::enableValidationPhase) {
        runValidationPhase();
    }
    runPredictionPhase();
    return;
}
