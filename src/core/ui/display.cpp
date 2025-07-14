// src/core/ui/display.cpp
/*
// --- Essential Includes ---
#include "display.h"         // Our display header (includes string, vector, button pins)
#include "esp_log.h"         // For ESP_LOGI, ESP_LOGE
#include "driver/gpio.h"
#include "driver/i2c_master.h"
// #include "driver/i2c.h" // Not needed if using i2c_master
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h" // For esp_lcd_new_panel_ssd1306
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "freertos/FreeRTOS.h" // For vTaskDelay
#include "freertos/task.h"     // For pdMS_TO_TICKS
#include <cstdio>              // For std::snprintf (C++ version)
// #include <cstring> // Not strictly needed with std::string and cstdio

// --- Global Variables and Constants (accessible to this compilation unit and externed in display.h) ---
lv_disp_t *g_main_disp = NULL; // Global LVGL display handle
static bool oled_is_functional = true;
static const char* TAG = "DISPLAY_MODULE"; // TAG for logging

// REMOVED: std::vector<std::string> log_buffer; // This caused "multiple definition". It's defined in utilities.cpp

// I2C Bus and Display Hardware Configuration (from display.h or explicitly defined here)
#define I2C_BUS_PORT                0
#define EXAMPLE_PIN_NUM_SDA         GPIO_NUM_3
#define EXAMPLE_PIN_NUM_SCL         GPIO_NUM_5
#define EXAMPLE_PIN_NUM_RST         -1           // -1 if no hardware reset pin is used/connected
#define EXAMPLE_I2C_HW_ADDR         0x3C         // Standard SSD1306 I2C address
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ  (400 * 1000) // 400kHz I2C speed
#define EXAMPLE_LCD_CMD_BITS        8
#define EXAMPLE_LCD_PARAM_BITS      8
 #define EXAMPLE_LCD_H_RES              128
 #define EXAMPLE_LCD_V_RES              64
 #define PIN_BTN_UP      GPIO_NUM_35
 #define PIN_BTN_DOWN    GPIO_NUM_34
 #define PIN_BTN_SELECT  GPIO_NUM_32
 #define MAX_LOG_LINES 7 
static void buttons_init();
static void delay_ms(int ms);
*/
// --- Utility Functions ---

static void delay_ms(int ms) {
    //vTaskDelay(pdMS_TO_TICKS(ms));
}

static void buttons_init() {
    /*
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL<<PIN_BTN_UP) | (1ULL<<PIN_BTN_DOWN) | (1ULL<<PIN_BTN_SELECT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);
    ESP_LOGI(TAG, "Buttons initialized.");
    */
}

// --- Display Initialization Function ---

void display_init() {
    /*
    ESP_LOGI(TAG, "Starting display initialization.");

    // 1. Initialize I2C bus
    i2c_master_bus_handle_t i2c_bus = NULL;
    // --- C++-compatible struct initialization (direct assignment to avoid 'out-of-order') ---
    i2c_master_bus_config_t bus_config;
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.i2c_port = I2C_BUS_PORT;
    bus_config.sda_io_num = EXAMPLE_PIN_NUM_SDA;
    bus_config.scl_io_num = EXAMPLE_PIN_NUM_SCL;
    bus_config.flags.enable_internal_pullup = true;
    bus_config.intr_priority = 0; // NECESSARY: Fixes 'invalid interrupt priority:12' runtime crash
    // --- END C++-compatible initialization ---

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));
    ESP_LOGI(TAG, "I2C master bus created successfully.");

    // 2. Install panel IO
    esp_lcd_panel_io_handle_t io_handle = NULL;
    // --- C++-compatible struct initialization (direct assignment) ---
    esp_lcd_panel_io_i2c_config_t io_config;
    io_config.dev_addr = EXAMPLE_I2C_HW_ADDR;
    io_config.scl_speed_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ;
    io_config.control_phase_bytes = 1;
    io_config.lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS;
    io_config.lcd_param_bits = EXAMPLE_LCD_CMD_BITS;
    io_config.dc_bit_offset = 6;
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
    io_config.flags.disable_control_phase = 1;
#endif
    // --- END C++-compatible initialization ---
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &io_handle));
    ESP_LOGI(TAG, "Panel IO created successfully.");

    // 3. Install SSD1306 panel driver
    esp_lcd_panel_handle_t panel_handle = NULL;
    // --- C++-compatible struct initialization (direct assignment) ---
    esp_lcd_panel_dev_config_t panel_config;
    panel_config.bits_per_pixel = 1;
    panel_config.reset_gpio_num = EXAMPLE_PIN_NUM_RST;
    // --- END C++-compatible initialization ---

#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
    // --- C++-compatible struct initialization for nested struct ---
    esp_lcd_panel_ssd1306_config_t ssd1306_config;
    ssd1306_config.height = EXAMPLE_LCD_V_RES;
    panel_config.vendor_config = &ssd1306_config;
    // --- END C++-compatible initialization ---
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));
    ESP_LOGI(TAG, "SSD1306 panel driver installed.");
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh1107(io_handle, &panel_config, &panel_handle));
    ESP_LOGI(TAG, "SH1107 panel driver installed.");
#else
#error "No LCD controller selected in Kconfig. Please enable CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306 in sdkconfig."
#endif

    // 4. Power on and initialize the panel
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_LOGI(TAG, "Panel reset complete.");
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_LOGI(TAG, "Panel initialization complete.");
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    ESP_LOGI(TAG, "Panel display ON.");

#if CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
#endif

    // 5. Initialize LVGL library
    ESP_LOGI(TAG, "Initializing LVGL library.");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);
    ESP_LOGI(TAG, "LVGL port initialized.");

    // 6. Add LVGL display to the port
    // --- C++-compatible struct initialization (direct assignment) ---
    lvgl_port_display_cfg_t disp_cfg;
    disp_cfg.io_handle = io_handle;
    disp_cfg.panel_handle = panel_handle;
    disp_cfg.buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES;
    disp_cfg.double_buffer = true;
    disp_cfg.hres = EXAMPLE_LCD_H_RES;
    disp_cfg.vres = EXAMPLE_LCD_V_RES;
    disp_cfg.monochrome = true;
    disp_cfg.rotation.swap_xy = false;
    disp_cfg.rotation.mirror_x = false;
    disp_cfg.rotation.mirror_y = false;
    // --- END C++-compatible initialization ---

    lv_disp_t *local_disp_handle = lvgl_port_add_disp(&disp_cfg);

    g_main_disp = local_disp_handle; // Assign to global handle

    lv_disp_set_rotation(g_main_disp, LV_DISP_ROT_NONE);
    ESP_LOGI(TAG, "LVGL display added and rotation set.");

    // Initialize buttons (if your project uses them)
    buttons_init();
    ESP_LOGI(TAG, "Display initialization complete. Ready for application logic.");
    */
}

// --- Display Content Functions ---

void display_clear() {
    /*
    if (g_main_disp == NULL) {
        ESP_LOGE(TAG, "Display handle is NULL, cannot clear.");
        return;
    }
    if (lvgl_port_lock(0)) {
        lv_obj_clean(lv_disp_get_scr_act(g_main_disp));
        lv_obj_set_style_bg_color(lv_disp_get_scr_act(g_main_disp), lv_color_black(), LV_PART_MAIN);
        lvgl_port_unlock();
    }
    ESP_LOGI(TAG, "Display cleared.");
    */
}

void display_update() {
    // Left empty as LVGL handles flushing in its dedicated task.
}

//void display_show_message(const std::string& title, const std::string& message, int delay_time_ms) {
    /*
    if (g_main_disp == NULL) {
        ESP_LOGE(TAG, "Display handle is NULL. display_init() might not have been called or failed.");
        return;
    }

    if (lvgl_port_lock(0)) {
        lv_obj_t *scr = lv_disp_get_scr_act(g_main_disp);
        lv_obj_clean(scr);

        lv_obj_set_style_bg_color(scr, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_text_color(scr, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_pad_all(scr, 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(scr, 0, LV_PART_MAIN);

        lv_obj_t *box = lv_obj_create(scr);
        lv_obj_set_size(box, EXAMPLE_LCD_H_RES / 2, EXAMPLE_LCD_V_RES / 2);
        lv_obj_set_style_bg_color(box, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_border_width(box, 0, LV_PART_MAIN);
        lv_obj_center(box);

        lv_obj_t *combined_label = lv_label_create(scr);
        char buffer[128];
        if (!title.empty()) {
            std::snprintf(buffer, sizeof(buffer), "%s\n%s", title.c_str(), message.c_str());
        } else {
            std::snprintf(buffer, sizeof(buffer), "%s", message.c_str());
        }
        lv_label_set_text(combined_label, buffer);
        lv_obj_set_style_text_font(combined_label, &lv_font_montserrat_12, LV_PART_MAIN);
        lv_obj_set_style_text_align(combined_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_center(combined_label);

        lvgl_port_unlock();
    }
    if (delay_time_ms > 0) {
        delay_ms(delay_time_ms);
    }
    ESP_LOGI(TAG, "Message displayed on screen (white background, black box).");
    */
//}

// --- UI Interaction Functions (LVGL implementations) ---

//int display_prompt_menu(const std::string& title, const std::vector<std::string>& items) {
    /*
    if (g_main_disp == NULL) {
        ESP_LOGE(TAG, "Display handle is NULL, cannot show menu.");
        return -1;
    }
    if (items.empty()) {
        ESP_LOGW(TAG, "display_prompt_menu called with empty items.");
        return -1;
    }

    int selected_idx = 0;
    lv_obj_t *menu_scr = NULL;
    lv_obj_t *list = NULL;

    if (lvgl_port_lock(0)) {
        menu_scr = lv_obj_create(NULL);
        lv_obj_set_size(menu_scr, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);
        lv_obj_set_style_bg_color(menu_scr, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_text_color(menu_scr, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_pad_all(menu_scr, 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(menu_scr, 0, LV_PART_MAIN);

        lv_obj_t *title_label = lv_label_create(menu_scr);
        lv_label_set_text(title_label, title.c_str());
        lv_obj_set_style_text_font(title_label, &lv_font_montserrat_12, LV_PART_MAIN);
        lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 2);

        list = lv_list_create(menu_scr);
        lv_obj_set_size(list, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES - 20);
        lv_obj_align(list, LV_ALIGN_BOTTOM_LEFT, 0, 0);
        lv_obj_set_style_bg_color(list, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_border_width(list, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_row(list, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_column(list, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_top(list, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(list, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_left(list, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_right(list, 0, LV_PART_MAIN);

        for (const auto& item_str : items) {
            lv_obj_t *btn = lv_list_add_btn(list, NULL, item_str.c_str());
            lv_obj_set_style_bg_color(btn, lv_color_black(), LV_PART_MAIN);
            lv_obj_set_style_text_color(btn, lv_color_white(), LV_PART_MAIN);
            lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
            lv_obj_set_style_radius(btn, 0, LV_PART_MAIN);
            lv_obj_set_style_pad_all(btn, 2, LV_PART_MAIN);
            lv_obj_set_style_text_font(btn, &lv_font_montserrat_10, LV_PART_MAIN);
        }
        lv_scr_load_anim(menu_scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        lvgl_port_unlock();
    }

    while (true) {
        bool up_pressed = !gpio_get_level(PIN_BTN_UP);
        bool down_pressed = !gpio_get_level(PIN_BTN_DOWN);
        bool select_pressed = !gpio_get_level(PIN_BTN_SELECT);

        if (down_pressed) {
            selected_idx = (selected_idx + 1) % items.size();
            delay_ms(200);
        }
        if (up_pressed) {
            selected_idx = (selected_idx - 1 + items.size()) % items.size();
            delay_ms(200);
        }
        ///////////////////////////// do forget to delete it 
       // select_pressed=true;
        if (select_pressed) {
            delay_ms(200);
            if (lvgl_port_lock(0)) {
                lv_obj_del(menu_scr);
                lv_scr_load_anim(lv_disp_get_scr_act(g_main_disp), LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
                lvgl_port_unlock();
            }
            ESP_LOGI(TAG, "Menu '%s' selected item: %d", title.c_str(), selected_idx);
            return selected_idx;
        }

        if (lvgl_port_lock(0)) {
            for (size_t i = 0; i < items.size(); ++i) {
                lv_obj_t *btn = lv_obj_get_child(list, i);
                if (btn) {
                    if (i == selected_idx) {
                        lv_obj_set_style_bg_color(btn, lv_color_white(), LV_PART_MAIN);
                        lv_obj_set_style_text_color(btn, lv_color_black(), LV_PART_MAIN);
                        lv_obj_scroll_to_view(btn, LV_ANIM_ON);
                    } else {
                        lv_obj_set_style_bg_color(btn, lv_color_black(), LV_PART_MAIN);
                        lv_obj_set_style_text_color(btn, lv_color_white(), LV_PART_MAIN);
                    }
                }
            }
            lvgl_port_unlock();
        }
        delay_ms(50);
    }
    */
//}


//bool display_prompt_yes_no(const std::string& title, const std::string& question) {
    /*
    std::vector<std::string> opts = {"Yes", "No"};
    ESP_LOGI(TAG, "Prompt: %s - %s", title.c_str(), question.c_str());
    //return false; ///////////////////////////////// uncomment this please
    return display_prompt_menu(question, opts) == 0;
    */
//}

void display_redraw_logs() {
    /*
    if (g_main_disp == NULL) {
        ESP_LOGE(TAG, "Display handle is NULL, cannot redraw logs.");
        return;
    }
    if (lvgl_port_lock(0)) {
        lv_obj_t *scr = lv_disp_get_scr_act(g_main_disp);
        lv_obj_clean(scr);
        lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);

        lv_obj_t *log_label_obj = lv_label_create(scr);
        lv_obj_set_size(log_label_obj, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);
        lv_obj_set_style_text_font(log_label_obj, &lv_font_montserrat_10, LV_PART_MAIN);
        lv_obj_set_style_text_color(log_label_obj, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_text_align(log_label_obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
        lv_obj_set_style_pad_all(log_label_obj, 2, LV_PART_MAIN);

        // Access log_buffer from utilities.cpp via extern declaration
        extern std::vector<std::string> log_buffer; // Declare extern here
        std::string full_log_text;
        for (const auto& line : log_buffer) {
            full_log_text += line + "\n";
        }
        lv_label_set_text(log_label_obj, full_log_text.c_str());

        lvgl_port_unlock();
    }
    */
}

//void display_add_log_line(const std::string& log_line) {
    /*
    // Access log_buffer from utilities.cpp via extern declaration
    extern std::vector<std::string> log_buffer; // Declare extern here
    log_buffer.push_back(log_line);
    if (log_buffer.size() > MAX_LOG_LINES) {
        log_buffer.erase(log_buffer.begin());
    }
        display_redraw_logs();

    ESP_LOGI(TAG, "LOG_ADDED: %s", log_line.c_str());
    */
//}

bool is_oled_functional(){
    return false;//oled_is_functional;
}

char display_wait_for_any_button() {
    /*if (g_main_disp == NULL) {
        ESP_LOGE(TAG, "Display handle is NULL, cannot wait for button.");
        return '\0';
    }

    lv_obj_t *wait_scr = NULL;
    if (lvgl_port_lock(0)) {
        wait_scr = lv_obj_create(NULL);
        lv_obj_set_size(wait_scr, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);
        lv_obj_set_style_bg_color(wait_scr, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_text_color(wait_scr, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_pad_all(wait_scr, 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(wait_scr, 0, LV_PART_MAIN);

        lv_obj_t *msg_label = lv_label_create(wait_scr);
        lv_label_set_text(msg_label, "Press any button to continue...");
        lv_obj_set_style_text_font(msg_label, &lv_font_montserrat_12, LV_PART_MAIN);
        lv_obj_align(msg_label, LV_ALIGN_CENTER, 0, 0);

        lv_scr_load_anim(wait_scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        lvgl_port_unlock();
    }

    while (true) {
        if (!gpio_get_level(PIN_BTN_UP)) { delay_ms(200); break; }
        if (!gpio_get_level(PIN_BTN_DOWN)) { delay_ms(200); break; }
        if (!gpio_get_level(PIN_BTN_SELECT)) { delay_ms(200); break; }
        delay_ms(50);
    }

    if (lvgl_port_lock(0)) {
        if (wait_scr != NULL) {
            lv_obj_del(wait_scr);
        }
        lv_scr_load_anim(lv_disp_get_scr_act(g_main_disp), LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        lvgl_port_unlock();
    }*/
    return 'Y';
}
