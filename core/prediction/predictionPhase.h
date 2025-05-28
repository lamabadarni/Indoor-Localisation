
 #include <../core/utils/utilities.h>  

#ifndef _IOT_INDOOR_LOCALISATION_PREDICTION_PHASE_H_
#define _IOT_INDOOR_LOCALISATION_PREDICTION_PHASE_H_


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


#endif // IOT_INDOOR_LOCALISATION_PREDICTION_PHASE_H
