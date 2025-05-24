#ifndef TOF_SCANNER_H
#define TOF_SCANNER_H

#include <Arduino.h>
#include "utillities.h"
#include "sdCardBackup.h"

/**
 * @brief Performs a batch of raw TOF scans (1 per responder × N scans).
 *        Each scan is stored separately (no averaging).
 */
void performTOFScan();

/**
 * @brief Computes how many TOF predictions match the scanning label.
 *        Each prediction is based on a new scan.
 * @return Number of successful predictions.
 */
int computeTOFPredictionMatches();

/**
 * @brief Creates a single TOF sample to be used for prediction.
 *        Returns a filled float[NUMBER_OF_RESPONDERS].
 */
void createTOFScanToMakePrediction(float out[NUMBER_OF_RESPONDERS]);

#endif // TOF_SCANNER_H
