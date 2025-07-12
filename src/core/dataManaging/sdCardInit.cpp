
#include "../utils/logger.h"
#include "soc/gpio_num.h"

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#define EXAMPLE_MAX_CHAR_SIZE    64

static const char *TAG = "SDCARD";

// Pin assignments for ESP32-S2
#define PIN_NUM_MISO  GPIO_NUM_35//35 39
#define PIN_NUM_MOSI  GPIO_NUM_33//33 35
#define PIN_NUM_CLK   GPIO_NUM_37 //37
#define PIN_NUM_CS    GPIO_NUM_18//12 5

bool initSDCard(void)
{
    LOG_INFO("SDCard", "Init SD Card.");
    esp_err_t ret;

    const char* mount_point = "/sdcard";
    sdmmc_card_t* card;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    LOG_DEBUG("SDCARD", "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = 10000;  // Lower frequency for stability

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = GPIO_NUM_35,
        .miso_io_num = GPIO_NUM_39,
        .sclk_io_num = GPIO_NUM_37,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize(static_cast<spi_host_device_t>(host.slot), &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        LOG_ERROR("SDCARD", "SPI bus init failed: %s (0x%x)", esp_err_to_name(ret), ret);
        return false;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = GPIO_NUM_5;
    slot_config.host_id = static_cast<spi_host_device_t>(host.slot);

    LOG_INFO("SDCARD", "Mounting filesystem...");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        LOG_ERROR("SDCARD", "esp_vfs_fat_sdspi_mount failed: %s (0x%x)", esp_err_to_name(ret), ret);

        if (ret == ESP_FAIL) {
            LOG_ERROR("SDCARD", "Mount failed. Card may not be FAT32 or not inserted correctly.");
        } else if (ret == ESP_ERR_INVALID_STATE) {
            LOG_ERROR("SDCARD", "SPI bus already in use or card not responding. Recheck wiring.");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            LOG_ERROR("SDCARD", "Card not detected. Check CS, MISO, MOSI, CLK, pull-ups.");
        }

        return false;
    }

    LOG_INFO("SDCARD", "Filesystem mounted successfully.");
    sdmmc_card_print_info(stdout, card);
    return true;
}
