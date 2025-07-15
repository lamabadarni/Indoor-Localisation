#include "core/utils/platform.h"
#include "core/utils/utilities.h"
#include "core/utils/logger.h"
#include "core/ui/userUI.h"
#include "core/ui/userUIOled.h"
#include "core/dataManaging/data.h"

#include "systemStateHandler/bootHandler.h"
#include "systemStateHandler/scanningSessionHandler.h"
#include "systemStateHandler/predictionSessionHandler.h"
#include "systemStateHandler/fullSessionHandler.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_vfs_dev.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

static void initLogger() {
    esp_log_level_set("*", ESP_LOG_INFO);    // ESP-IDF default
    esp_vfs_dev_uart_use_driver(0); // Enable UART0 for stdin/stdout
    setvbuf(stdin, NULL, _IONBF, 0); // Unbuffered stdin
    SystemSetup::logLevel = LOG_LEVEL_INFO;  // Force your own log level system
}

static void handleSoftExit() {
    LOG_INFO("MAIN", "Session ended. Goodbye.");
    fclose(logFile);
}

static void initWiFi() {
    /* 1. Non-volatile storage ------------------------------------------------ */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    /* 2. **TCP/IP stack + SYSTEM EVENT LOOP -------------------------*/
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* 3. Wi-Fi driver -------------------------------------------------------- */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));   // or WIFI_MODE_APSTA
    ESP_ERROR_CHECK(esp_wifi_start());                   // driver is now up
}
extern "C" {
    void display_init_helper(void);
}

static void printStartupBanner() {
    LOG_INFO("MAIN", "  ╔══════════════════════════════════════════════════════╗\n");
    LOG_INFO("MAIN", "  ║                                                      ║\n");
    LOG_INFO("MAIN", "  ║     📍  Indoor Localization System - ESP32-S2        ║\n");
    LOG_INFO("MAIN", "  ║                                                      ║\n");
    LOG_INFO("MAIN", "  ║     ┌────────┐   ┌────────┐   ┌────────┐              ║\n");
    LOG_INFO("MAIN", "  ║     │ Anchor │―▶ │ Signal │―▶ │  Scan  │              ║\n");
    LOG_INFO("MAIN", "  ║     └────────┘   └────────┘   └────────┘              ║\n");
    LOG_INFO("MAIN", "  ║         ▲             ▲             ▲                ║\n");
    LOG_INFO("MAIN", "  ║       [RSSI]       [TOF]        [LABELING]            ║\n");
    LOG_INFO("MAIN", "  ║                                                      ║\n");
    LOG_INFO("MAIN", "  ║      🗺️  Mapping, Prediction, Validation...           ║\n");
    LOG_INFO("MAIN", "  ║                                                      ║\n");
    LOG_INFO("MAIN", "  ╚══════════════════════════════════════════════════════╝\n");
}

extern "C" void app_main() {
    initLogger();     // Replace Serial.begin() for ESP-IDF logging
    printStartupBanner();
    bool initSD = true;

    delay_ms(1000);  // Optional startup delay

    initWiFi();

    while (true) {
        
        shouldAbort = false;

        runUserSystemSetup();  // Prompts user for system mode and settings

        delay_ms(1000); 

        if(SystemSetup::currentSystemMode != MODE_SYSTEM_BOOT && !initDataBackup(initSD)) {
             LOG_ERROR("MAIN", "Failed to initialize SD card or log file.");
        }

        initSD = false;

        switch (SystemSetup::currentSystemMode) {
            case MODE_SYSTEM_BOOT:
                handleSystemBoot();
                break;
            case MODE_SCANNING_SESSION:
                handleScanningSession();
                break;
            case MODE_PREDICTION_SESSION:
                handlePredictionSession();
                break;
            case MODE_FULL_SESSION:
                handleFullSession();
                break;
            default:
                LOG_ERROR("MAIN", "Unknown system mode. Exiting.");
                break;
        }

        char again = promptUserRunAnotherSession();
        if (!again) {
            break;
        }
    }
    handleSoftExit();
}
