
#include "../core/utils/platform.h"
#include "../core/utils/utilities.h"
#include "../core/ui/userUI.h"
#include "../core/ui/logger.h"

#include "systemStateHandler/bootHandler.h"
#include "systemStateHandler/scanningSessionHandler.h"
#include "systemStateHandler/predectionSessionHandler.h"
#include "systemStateHandler/fullSessionHandler.h"

#include "esp_event.h"
#include "esp_log.h"

static void initLogger() {
    // Set the global log level (applies to all tags unless overridden)
    esp_log_level_set("*", ESP_LOG_INFO);
}

static void handleSoftExit() {
    
}


extern "C" void app_main() {
    initLogger();  // Replace Serial.begin() for ESP-IDF logging
    delay_ms(10000);  // Optional startup delay

    while (true) {
        runUserSystemSetup();  // Prompts user for system mode and settings

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
                LOG_ERROR("MAIN", "Unknown system mode.");
                break;
        }


        if(reconfigure) break;
        if(shouldAbort) handleSoftExit();

        LOG_INFO("MAIN", "Would you like to run another session?");
        LOG_INFO("MAIN", "(y - yes | n - no)");
        char again = readCharFromUser();
        if (again != 'y' && again != 'Y') {
            LOG_INFO("MAIN", "Session ended. Goodbye.");
            break;
        }
    }
}
