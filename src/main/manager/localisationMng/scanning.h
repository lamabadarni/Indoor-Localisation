/**
 * @file tofScanner.h
 * @brief TOF (Time-of-Flight) scanning interface for collecting responder distances and validation input.
 *
 * Handles asynchronous distance measurements using WiFi FTM sessions and organizes prediction-ready input arrays.
 * Supports both prediction phase scans and multi-attempt validation rounds.
 * 
 * @author Lama Badarni
 */


#ifndef IOT_INDOOR_LOCALISATION_SCANNING_H
#define IOT_INDOOR_LOCALISATION_SCANNING_H

#include <Arduino.h>
#include <WiFi.h>

/**
 * @brief Executes the scanning phase across all location labels.
 *
 * For each label, prompts the user to begin scanning, collects RSSI and/or TOF data
 * based on the current system state, validates the scan, and repeats if necessary.
 * This function is typically used to gather training or prediction data for the localization model.
 */
void runScanningPhase();

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
