
#ifndef _SCANNINGPHASE_
#define _SCANNINGPHASE_

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


void scanTOF();

void rescan();

//double computeScanAccuracy();

void createSingleScan();


#endif //IOT_INDOOR_LOCALISATION_SCANNING_H
