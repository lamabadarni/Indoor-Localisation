#ifndef IOT_INDOOR__RSSI_LOCALISATION_SCANNING_H
#define IOT_INDOOR__RSSI_LOCALISATION_SCANNING_H

#include <Arduino.h>
#include <WiFi.h>
#include "utillities.h"
#include "sdCardBackup.h"

/**
 * @brief Starts a scanning session at a given label with retries and validation.
 * @param label The target label to scan at.
 * @return true if scan was successful and accepted, false otherwise.
 */
bool startLabelScanningSession(Label label);

/**
 * @brief Collects RSSI/TOF data based on the current system state.
 *        Dispatches to appropriate scanning functions.
 */
void collectMeasurements();

/**
 * @brief Validates prediction accuracy and asks for user approval.
 *        Includes fallback to RSSI-only or TOF-only if combined accuracy fails.
 * @return true if validation succeeded, false if aborted or rejected.
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

/**
 * @brief Global scan accuracy percentage computed after validation.
 */

#endif // IOT_INDOOR__RSSI_LOCALISATION_SCANNING_H
