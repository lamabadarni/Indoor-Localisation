/*
    * kNN.h
    *
    * Created on: 2023.10.01
    * @author: Ward Iraqi
    * @brief: Header file for kNN algorithm implementation.
    * This file contains the function declarations and constants used in the kNN algorithm.
    * The kNN algorithm is used for predicting the location based on RSSI values.
*/

#ifndef IOT_INDOOR_LOCALISATION_predectionPhase_H
#define IOT_INDOOR_LOCALISATION_predectionPhase_H

#include <Arduino.h>
#include <math.h>
#include <WiFi.h>
#include "utillities.h"

/*
    * @brief Predicts the location using the kNN algorithm based on input RSSI values.
    * @param input: sample array of RSSI values
    * @return: predicted location
*/
LOCATIONS rssiPredict(double input[NUMBER_OF_ANCHORS]);

/*
    * @brief Predicts the location using the kNN algorithm based on input TOF values.
    * @param input: sample array of TOF values
    * @return: predicted location
*/
LOCATIONS tofPredict(double input[NUMBER_OF_RESPONDERS]);

/*
    * @brief Prepares the RSSI data for prediction.
    * @param RSSIs: Array of RSSI values to be prepared.
*/
void preparePoint(double RSSIs[NUMBER_OF_ANCHORS]);

/*
    * @brief Determines the relevance of the backup dataset.
    *        Evaluates the dataset based on prediction accuracy.
    *        only called at setup() assumes that dataset is already filled
    *        with the backup dataset.
    * @return: True if the backup dataset meets the relevance criteria, false otherwise.
*/
bool isBackupDataSetRelevant(void);

LOCATIONS validateScanningPhasePerLabel(std::vector<Data> scanResultSamepleForValidationPerLabel);

#endif