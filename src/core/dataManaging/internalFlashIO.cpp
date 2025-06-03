
#include "internalFlashIO.h"
#include "esp_littlefs.h"


// ====================== Initialization ======================

// Initialize and mount the “storage” partition on internal flash.
// Returns true if mount (or successful format+mount) succeeded.
bool initInternalFlash() {
    static constexpr const char* PART_LABEL   = "storage";   // must match partitions.csv
    static constexpr const char* MOUNT_POINT  = "/littlefs";

    esp_vfs_littlefs_conf_t conf = {
        .partition_label        = PART_LABEL,
        .base_path              = MOUNT_POINT,
        .format_if_mount_failed = false,       // don’t auto-format
        .dont_mount             = false,       // mount immediately
        .max_files              = 5            // adjust as needed
    };

    // 1) Try to mount
    esp_err_t err = esp_vfs_littlefs_register(&conf);
    if (err == ESP_OK) {
        size_t total = 0, used = 0;
        ESP_ERROR_CHECK( esp_vfs_littlefs_info(PART_LABEL, &total, &used) );
        LOG_INFO("FLASH",
                 "Mounted '%s' at '%s' (total: %u bytes, used: %u bytes)",
                 PART_LABEL, MOUNT_POINT,
                 static_cast<unsigned>(total),
                 static_cast<unsigned>(used));
        return true;
    }

    // 2) Mount failed, warn
    LOG_WARN("FLASH",
             "LittleFS mount failed on partition '%s': %s",
             PART_LABEL, esp_err_to_name(err));

    // 3) Ask user whether to format
    if (!promptUserFlashRecoveryApprove()) {
        LOG_ERROR("FLASH", "User declined flash recovery.");
        return false;
    }

    // 4) Format and re-mount
    LOG_INFO("FLASH", "Formatting partition '%s'...", PART_LABEL);
    err = esp_vfs_littlefs_format(PART_LABEL);
    if (err != ESP_OK) {
        LOG_ERROR("FLASH",
                  "Format failed on '%s': %s",
                   PART_LABEL, esp_err_to_name(err));
        return false;
    }

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