//
// Created by Lama Badarni on 13/05/2025.
//

#ifndef IOT_INDOOR_LOCALISATION_SCANNING_H
#define IOT_INDOOR_LOCALISATION_SCANNING_H

#include <Arduino.h>
#include <WiFi.h>
#include "utillities.h"

/**
 *  @brief Starts a scanning session at a given label with retries and validation.
 *  @param label The target label to scan at.
 *  @return true if scan was successful and accepted, false otherwise.
*/
bool startLabelScanningSession();

/**
 * @brief Collects RSSI/TOF data based on the current system state.
 *        Dispatches to appropriate scanning functions.
 */
void collectMeasurements();

/**
 * @brief Validates prediction accuracy and asks for user approval.
 * @param matches Number of correct predictions.
 * @return true if user approves the scan, false otherwise.
 */
bool validateScanAccuracy();

/**
 * @brief Performs static RSSI scanning from fixed anchors.
 */
void scanStaticRSSI();

/**
 * @brief Scans additional RSSI sources (future: dynamic RSSI).
 */
void scanDynamicRSSI();


/**
 * @brief Scans Time-of-Flight (TOF) distances from responders.
 */
void scanTOF();


#endif //IOT_INDOOR_LOCALISATION_SCANNING_H
