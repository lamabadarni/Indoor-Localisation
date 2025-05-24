/**
 * @file rssiScanner.h
 * @brief Interface for performing RSSI-based scanning, prediction, and data collection.
 * 
 * Functions support both batch-based training scan collection and single-sample prediction.
 * Used in localization workflows where RSSI is measured from known static Wi-Fi anchors.
 */

#ifndef RSSI_SCANNER_H
#define RSSI_SCANNER_H

#include <Arduino.h>
#include "utillities.h"
#include "sdCardBackup.h"

/**
 * @brief Executes the scanning phase across all location labels.
 *
 * For each label, prompts the user to begin scanning, collects RSSI and/or TOF data
 * based on the current system state, validates the scan, and repeats if necessary.
 * This function is typically used to gather training or prediction data for the localization model.
 */
void runScanningPhase();

/**
 * @brief Performs a batch of RSSI scans and stores them into the global dataset.
 *        Each scan is a smoothed sample using EMA across N RSSI samples.
 */
void performRSSIScan();

/**
 * @brief Creates a single RSSI scan input for prediction.
 *        This scan uses EMA across RSSI_SCAN_SAMPLE_PER_BATCH samples.
 *        and fills accumlatedRSSIs global with sampled RSSIs
 */
void createRSSIScanToMakePrediction();

/**
 * @brief Performs NUM_OF_VALIDATION_SCANS predictions and compares them to the current label.
 *        Adds successful prediction samples to the dataset.
 * @return Number of correct predictions.
 */
int computeRSSIPredictionMatches();

#endif // RSSI_SCANNER_H
