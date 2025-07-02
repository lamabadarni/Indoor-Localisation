#pragma once

#ifdef __cplusplus
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>
#include <string.h>
#include <cstdarg>
#include <cstdio>
#include <cstdio> 
#endif
#include <stdio.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>


#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

#define delay_ms(ms) vTaskDelay((ms) / portTICK_PERIOD_MS)
#define millis_since_boot() (esp_timer_get_time() / 1000)
#define log_info(tag, msg) ESP_LOGI(tag, "%s", msg)
#define log_inline(tag, msg) ESP_LOGI(tag, "%s", msg)
#endif