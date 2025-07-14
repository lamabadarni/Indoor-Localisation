#ifndef _IOT_INDOOR_LOCALISATION_PREDICTION_PHASE_H_
#define _IOT_INDOOR_LOCALISATION_PREDICTION_PHASE_H_

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "core/ui/userUI.h"  
#include "core/ui/userUIOled.h"  

void runPredictionPhase();

Label predict();

bool startLabelPredectionSession();


#endif // IOT_INDOOR_LOCALISATION_PREDICTION_PHASE_H
