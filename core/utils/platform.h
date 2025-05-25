#pragma once

#ifdef ARDUINO

  #include <Arduino.h>
  #include <WiFi.h>
  #include <SD.h>
  #include <SPI.h>

  #define delay_ms(ms) delay(ms)
  #define millis_since_boot() millis()
  #define log_info(tag, msg) Serial.println(String("[") + tag + "] " + msg)
  #define log_inline(tag, msg) Serial.print(String("[") + tag + "] " + msg)

#else

  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #include "esp_log.h"
  #include "esp_timer.h"
  #include "esp_wifi.h"
  #include "esp_event.h"
  #include "esp_vfs_fat.h"
  #include "sdmmc_cmd.h"
  #include "driver/sdmmc_host.h"
  #include <string.h>

  #define delay_ms(ms) vTaskDelay((ms) / portTICK_PERIOD_MS)
  #define millis_since_boot() (esp_timer_get_time() / 1000) // microseconds to ms
  #define log_info(tag, msg) ESP_LOGI(tag, "%s", msg)
  #define log_inline(tag, msg) ESP_LOGI(tag, "%s", msg)

#endif

