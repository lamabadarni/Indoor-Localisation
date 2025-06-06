#ifndef _IOT_INDOOR_LOCALISATION_PREDICTION_PHASE_H_
#define _IOT_INDOOR_LOCALISATION_PREDICTION_PHASE_H_

#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "core/ui/userUI.h"  

/**
 * @brief this function predicts the current label based on RSSI and ToF values accumualted in real time.
 */
void runPredictionPhase(void);

/*
    * @brief Predicts the location using the kNN algorithm based on input RSSI values.
    * @return: predicted location
*/
Label rssiPredict();

/*
    * @brief Predicts the location using the kNN algorithm based on input TOF values.
    * @return: predicted location
*/
Label tofPredict();

Label predict();

bool startLabelPredectionSession();

bool clearDataAfterPredectionFailure();

Label createSamplePredict();

void printPredictionSummary();


#endif // IOT_INDOOR_LOCALISATION_PREDICTION_PHASE_H
