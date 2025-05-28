#pragma once
#ifdef __cplusplus
#include <string>
#include <cstring>
#include <vector>
#include <cstdarg>
#include <cstdio>
#endif
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <math.h>


#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
//#include "esp_event.h"
//#include "nvs_flash.h"
//#include "sdmmc_cmd.h"
//#include "driver/sdmmc_host.h"

#define delay_ms(ms) vTaskDelay((ms) / portTICK_PERIOD_MS)
#define millis_since_boot() (esp_timer_get_time() / 1000)
#define log_info(tag, msg) ESP_LOGI(tag, "%s", msg)
#define log_inline(tag, msg) ESP_LOGI(tag, "%s", msg)
#endif