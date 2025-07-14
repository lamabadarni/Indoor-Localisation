
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h" // For esp_lcd_new_panel_ssd1306
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "display.h"
// #include "driver/i2c.h"            // ESP32-S2 I2C driver (not strictly needed if using i2c_master)
#include "esp_log.h"
#include "freertos/FreeRTOS.h" // Needed for vTaskDelay
#include "freertos/task.h"     // Needed for pdMS_TO_TICKS
// Global LVGL display handle
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_lvgl_port.h"
// Global LVGL objects for log display
static lv_obj_t *log_screen = NULL;
static lv_obj_t *log_label = NULL; // Using a label for simplicity to manually manage lines
static const char* TAG = "DISPLAY_TEST";

static bool oled_is_functional = true;

// Global handle for the LVGL display, accessible by other functions
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
#include "esp_lcd_sh1107.h"
#else
#include "esp_lcd_panel_vendor.h"
#endif


#define I2C_BUS_PORT  0

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ    (400 * 1000)
#define EXAMPLE_PIN_NUM_SDA           3
#define EXAMPLE_PIN_NUM_SCL           5
#define EXAMPLE_PIN_NUM_RST           -1
#define EXAMPLE_I2C_HW_ADDR           0x3C

// The pixel number in horizontal and vertical
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
#define EXAMPLE_LCD_H_RES              128
#define EXAMPLE_LCD_V_RES              CONFIG_EXAMPLE_SSD1306_HEIGHT
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
#define EXAMPLE_LCD_H_RES              64
#define EXAMPLE_LCD_V_RES              128
#endif
// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8

#define PIN_BTN_UP      GPIO_NUM_35
#define PIN_BTN_DOWN    GPIO_NUM_34
#define PIN_BTN_SELECT  GPIO_NUM_32
#include "lvgl.h"



// Simple button inputs with pull-ups (retained as polling mechanism)
void delay_ms(int ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}
static void buttons_init() {
    gpio_config_t cfg = {};
    cfg.intr_type    = GPIO_INTR_DISABLE;
    cfg.mode         = GPIO_MODE_INPUT;
    cfg.pin_bit_mask = (1ULL<<PIN_BTN_UP) |
                       (1ULL<<PIN_BTN_DOWN) |
                       (1ULL<<PIN_BTN_SELECT);
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.pull_up_en   = GPIO_PULLUP_ENABLE;
    gpio_config(&cfg);
}

void display_init_helper() {
    // Initialize I2C bus
    
    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_master_bus_handle_t i2c_bus = NULL;
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .i2c_port = I2C_BUS_PORT,
        .sda_io_num = EXAMPLE_PIN_NUM_SDA,
        .scl_io_num = EXAMPLE_PIN_NUM_SCL,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));
    // Add logging here: ESP_LOGI(TAG, "I2C master bus created successfully.");

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = EXAMPLE_I2C_HW_ADDR,
        .scl_speed_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .control_phase_bytes = 1,               // According to SSD1306 datasheet
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,   // According to SSD1306 datasheet
        .lcd_param_bits = EXAMPLE_LCD_CMD_BITS, // According to SSD1306 datasheet
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
        .dc_bit_offset = 6,                     // According to SSD1306 datasheet
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
        .dc_bit_offset = 0,                     // According to SH1107 datasheet
        .flags =
        {
            .disable_control_phase = 1,
        }
#endif
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &io_handle));
    // Add logging here: ESP_LOGI(TAG, "Panel IO created successfully.");

    ESP_LOGI(TAG, "Install SSD1306 panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
    };
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
    esp_lcd_panel_ssd1306_config_t ssd1306_config = {
        .height = EXAMPLE_LCD_V_RES,
    };
    panel_config.vendor_config = &ssd1306_config;
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));
    // Add logging here: ESP_LOGI(TAG, "SSD1306 panel driver installed.");
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh1107(io_handle, &panel_config, &panel_handle));
    // Add logging here: ESP_LOGI(TAG, "SH1107 panel driver installed.");
#endif

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    // Add logging here: ESP_LOGI(TAG, "Panel reset complete.");
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    // Add logging here: ESP_LOGI(TAG, "Panel initialization complete.");
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    // Add logging here: ESP_LOGI(TAG, "Panel display ON.");

#if CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
#endif

    ESP_LOGI(TAG, "Initialize LVGL");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);
    // Add logging here: ESP_LOGI(TAG, "LVGL port initialized.");

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES,
        .double_buffer = true,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = true,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        }
    };
    lv_disp_t *disp = lvgl_port_add_disp(&disp_cfg);

    // === START OF REQUIRED CHANGE ===
    // Assign the local 'disp' handle to the global 'g_main_disp' variable
    g_main_disp = disp;
    // === END OF REQUIRED CHANGE ===

    lv_disp_set_rotation(disp, LV_DISP_ROT_NONE);
    // Add logging here: ESP_LOGI(TAG, "LVGL display added and rotation set.");

    ESP_LOGI(TAG, "Display LVGL Scroll Text");
    
    // Lock the mutex due to the LVGL APIs are not thread-safe
   // if (lvgl_port_lock(0)) {
     //   lvgl_demo_ui(disp);
        // Release the mutex
    //    lvgl_port_unlock();
    //}
    
    // Also, ensure the commented-out block above (with example_lvgl_demo_ui) remains commented out or is removed.
    // This will prevent it from conflicting with your custom display_show_message.

    // Add logging and calls for buttons_init() if desired here,
    // similar to the full display.c I provided in the "write me it all over" response.
    // Example:
    // buttons_init();
    // ESP_LOGI(TAG, "Buttons initialized.");
    // ESP_LOGI(TAG, "Display initialization complete. Ready for application logic.");
}