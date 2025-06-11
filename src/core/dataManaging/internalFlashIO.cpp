#include "internalFlashIO.h"
#include "../utils/logger.h"
#include "esp_littlefs.h" // Use this for the ESP-IDF LittleFS component


#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
//#include "esp_vfs_fat.h"
//#include "sdmmc_cmd.h"
#include "../utils/logger.h"    // your own logger
#include "../ui/userUI.h"    // your own logger
#include "esp_littlefs.h"
#define EXAMPLE_MAX_CHAR_SIZE    64


//#define MOUNT_POINT "/sdcard"

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO  CONFIG_EXAMPLE_PIN_MISO
#define PIN_NUM_MOSI  CONFIG_EXAMPLE_PIN_MOSI
#define PIN_NUM_CLK   CONFIG_EXAMPLE_PIN_CLK
#define PIN_NUM_CS    CONFIG_EXAMPLE_PIN_CS



// ====================== Initialization ======================
bool initInternalFlash()
{
    static constexpr const char* PART_LABEL   = "storage";   // must match partitions.csv

    // Zero‐initialize the struct, then assign each field by name.
    esp_vfs_littlefs_conf_t conf{};
    conf.partition_label        = PART_LABEL;
    conf.base_path              = MOUNT_POINT;
    conf.format_if_mount_failed = false;
    conf.dont_mount             = false;
    // ----- In your version of esp_vfs_littlefs_conf_t, there is NO "max_files" field. 
    // If your installed header actually defines max_files, uncomment the next line:
    // conf.max_files              = 5;

    // 1) Attempt to register (mount) LittleFS
    esp_err_t err = esp_vfs_littlefs_register(&conf);
    if (err == ESP_OK) {
        // We are successfully mounted. Now query usage stats:
        size_t total = 0, used = 0;
        ESP_ERROR_CHECK( esp_littlefs_info(PART_LABEL, &total, &used) );
        LOG_INFO("FLASH",
                 "Mounted '%s' at '%s' (total: %u bytes, used: %u bytes)",
                 PART_LABEL, MOUNT_POINT,
                 static_cast<unsigned>(total),
                 static_cast<unsigned>(used));
        return true;
    }

    // 2) Mount failed; log why
    LOG_WARN("FLASH",
             "LittleFS mount failed on partition '%s': %s",
             PART_LABEL, esp_err_to_name(err));

    // 3) Ask the user for permission to reformat
    if (!promptUserReuseDecision()) { /////////////NEED TO CHECK THIS LINE 
        LOG_ERROR("FLASH", "User declined flash recovery.");
        return false;
    }

    // 4) Format and re‐mount
    LOG_INFO("FLASH", "Formatting partition '%s'...", PART_LABEL);
    err = esp_littlefs_format(PART_LABEL);
    if (err != ESP_OK) {
        LOG_ERROR("FLASH",
                  "Format failed on '%s': %s",
                  PART_LABEL, esp_err_to_name(err));
        return false;
    }

    // Now that it’s formatted, try mounting again:
    err = esp_vfs_littlefs_register(&conf);
    if (err != ESP_OK) {
        LOG_ERROR("FLASH",
                  "Re-mount after format failed on '%s': %s",
                  PART_LABEL, esp_err_to_name(err));
        return false;
    }

    LOG_INFO("FLASH", "Recovery successful; LittleFS mounted at '%s'.", MOUNT_POINT);
    return true;
}