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


extern "C" void app_main() {
    initLogger();     // Replace Serial.begin() for ESP-IDF logging
    LOG_INFO("MAIN", "START");

    bool initSD = true;

    delay_ms(1000);  // Optional startup delay

    while (true) {
        
        shouldAbort = false;

        runUserSystemSetup();  // Prompts user for system mode and settings

        if(!initDataBackup(initSD)) {
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
  
        if(shouldAbort) break;

        char again = promptUserRunAnotherSession();
        if (!again) {
            break;
        }
    }
    handleSoftExit();
}
