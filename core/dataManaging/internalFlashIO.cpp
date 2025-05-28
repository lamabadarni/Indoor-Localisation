
#include "internalFlashIO.h"
#include "../utils/platform.h"
#include "../ui/logger.h"
#include "../utils/utilities.h"
#include "../ui/userUI.h"
#include "esp_littlefs.h"


// ====================== Initialization ======================

bool initInternalFlash() {
    esp_vfs_littlefs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .format_if_mount_failed = false
    };

    esp_err_t err = esp_vfs_littlefs_register(&conf);
    if (err == ESP_OK) {
        size_t total, used;
        esp_littlefs_info(NULL, &total, &used);
        LOG_INFO("FLASH", "LittleFS mounted. Total: %d, Used: %d", total, used);
        return true;
    }

    LOG_WARN("FLASH", "LittleFS mount failed: %s", esp_err_to_name(err));

    if (!promptUserFlashRecoveryApprove()) {
        LOG_ERROR("FLASH", "LittleFS recovery declined by user.");
        return false;
    }

    LOG_INFO("FLASH", "Formatting LittleFS...");
    if (esp_littlefs_format(NULL) != ESP_OK) {
        LOG_ERROR("FLASH", "LittleFS format failed.");
        return false;
    }

    if (esp_vfs_littlefs_register(&conf) != ESP_OK) {
        LOG_ERROR("FLASH", "LittleFS re-mount failed after format.");
        return false;
    }

    LOG_INFO("FLASH", "LittleFS recovery successful.");
    return true;
}