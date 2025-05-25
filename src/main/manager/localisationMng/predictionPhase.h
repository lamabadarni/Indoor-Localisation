/**
 * @file predictionPhase.h
 * @brief Prediction system interface for computing locations using RSSI and TOF kNN models.
 *
 * Declares methods to run predictions and collect scan input using global buffers.
 * Used in both real-time location estimation and validation scenarios.
 * 
 * @author Ward Iroq
 */


#ifndef _IOT_INDOOR_LOCALISATION_PREDICTION_PHASE_H_
#define _IOT_INDOOR_LOCALISATION_PREDICTION_PHASE_H_

#include <Arduino.h>
#include <math.h>
#include <WiFi.h>
#include <utilities.h>

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
