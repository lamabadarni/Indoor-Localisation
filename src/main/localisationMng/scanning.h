//
// Created by Lama Badarni on 13/05/2025.
//

#ifndef IOT_INDOOR_LOCALISATION_SCANNING_H
#define IOT_INDOOR_LOCALISATION_SCANNING_H

#include <Arduino.h>
#include <WiFi.h>
#include "utillities.h"


/**
 * @brief Collects RSSI data for the specified location.
 * @param locationLabel The label of the location.
 */
void performScan(LOCATIONS locationLabel);

int applyEMA(int prevRSSI, int newRSSI);
 
/**
 * @brief Interactive prompt for selecting a location.
 * @return Selected location as LOCATIONS enum.
 */
LOCATIONS promptLocationSelection();


#endif //IOT_INDOOR_LOCALISATION_SCANNING_H
