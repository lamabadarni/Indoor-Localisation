
#include "internalFlashIO.h"
#include "../utils/logger.h"


/* SD card and FAT filesystem example.
   This example uses SPI peripheral to communicate with SD card.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#define EXAMPLE_MAX_CHAR_SIZE    64

static const char *TAG = "example";

#define MOUNT_POINT "/sdcard"

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO  CONFIG_EXAMPLE_PIN_MISO
#define PIN_NUM_MOSI  CONFIG_EXAMPLE_PIN_MOSI
#define PIN_NUM_CLK   CONFIG_EXAMPLE_PIN_CLK
#define PIN_NUM_CS    CONFIG_EXAMPLE_PIN_CS

void app_main(void)
{
    esp_err_t ret;

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    LOG_DEBUG(TAG, "Initializing SD card");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.
    LOG_DEBUG(TAG, "Using SPI peripheral");

    // By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
    // For setting a specific frequency, use host.max_freq_khz (range 400kHz - 20MHz for SDSPI)
    // Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 20000,
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        LOG_DEBUG(TAG, "Failed to initialize bus.");
        return;
    }

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            LOG_DEBUG(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            LOG_DEBUG(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    LOG_DEBUG(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Use POSIX and C standard library functions to work with files.
}

/*
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
    */