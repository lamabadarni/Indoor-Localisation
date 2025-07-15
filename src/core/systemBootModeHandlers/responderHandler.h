#include <string.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "core/utils/logger.h"
#include "esp_system.h"
#include "esp_mac.h" 

#ifndef TOF_RESPONDER_H
#define TOF_RESPONDER_H

/**
 * @brief Initializes this ESP32-S2 device as an FTM responder in AP mode.
 * Call from app_main() for responder devices.
 */
void initFTMResponder();

#endif // TOF_RESPONDER_H
