// display.cpp
#include "display.h"

#include <u8g2.h>
#include <u8g2_esp32_hal.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "../utils/platform.h"   // for delay_ms()
#include "driver/i2c_master.h"
#include "../utils/utilities.h"

// U8g2 core + HAL instances
static u8g2_t           u8g2;
static u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
static bool oled_is_functional = true;

// Simple button inputs with pull-ups
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


void display_init() {
    buttons_init();
    u8g2_esp32_hal.bus.i2c.sda = OLED_I2C_SDA;
    u8g2_esp32_hal.bus.i2c.scl = OLED_I2C_SCL;
    u8g2_esp32_hal_init(u8g2_esp32_hal); // This function is provided by the u8g2-hal-esp-idf library

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(
      &u8g2,
      U8G2_R0,
      u8g2_esp32_i2c_byte_cb,
      u8g2_esp32_gpio_and_delay_cb
    );
    u8x8_SetI2CAddress(&u8g2.u8x8, 0x78);

    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    display_clear();
    display_update();
}



void display_clear() {
    u8g2_ClearBuffer(&u8g2);
}

void display_update() {
    u8g2_SendBuffer(&u8g2);
}

void display_show_message(const char* title, const char* message, int delay_ms) {
    display_clear();

    // Draw title (larger font)
    u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
    int tw = u8g2_GetStrWidth(&u8g2, title);
    int tx = (u8g2_GetDisplayWidth(&u8g2) - tw)/2;
    u8g2_DrawStr(&u8g2, tx, 18, title);

    // Draw message (smaller font)
    u8g2_SetFont(&u8g2, u8g2_font_ncenR08_tr);
    int mw = u8g2_GetStrWidth(&u8g2, message);
    int mx = (u8g2_GetDisplayWidth(&u8g2) - mw)/2;
    u8g2_DrawStr(&u8g2, mx, 40, message);

    display_update();
    if (delay_ms > 0) {
        delay_ms(delay_ms);
    }
}

int display_prompt_menu(const std::string& title, const std::vector<std::string>& items) {
    if (items.empty()) return -1;
    int sel = 0;
    const int line_h = 12;
    const int y0     = 24;
    const int max_lines = (64 - y0) / line_h;

    while (true) {
        // Read buttons (active-low)
        if (!gpio_get_level(PIN_BTN_DOWN)) {
            sel = (sel + 1) % items.size();
            delay_ms(200);
        }
        if (!gpio_get_level(PIN_BTN_UP)) {
            sel = (sel - 1 + items.size()) % items.size();
            delay_ms(200);
        }
        if (!gpio_get_level(PIN_BTN_SELECT)) {
            delay_ms(200);
            return sel;
        }

        // Draw menu
        display_clear();
        // Title
        u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
        u8g2_DrawStr(&u8g2, 0, 10, title.c_str());
        u8g2_DrawHLine(&u8g2, 0, 13, u8g2_GetDisplayWidth(&u8g2));

        // List items
        u8g2_SetFont(&u8g2, u8g2_font_profont12_tr);
        int start = sel < max_lines ? 0 : sel - max_lines + 1;
        for (int i = 0; i < max_lines; ++i) {
            int idx = start + i;
            if (idx >= (int)items.size()) break;
            int y = y0 + i*line_h;
            if (idx == sel) {
                u8g2_DrawBox(&u8g2, 0, y - line_h + 2,
                             u8g2_GetDisplayWidth(&u8g2), line_h);
                u8g2_SetDrawColor(&u8g2, 0);
                u8g2_DrawStr(&u8g2, 2, y, items[idx].c_str());
                u8g2_SetDrawColor(&u8g2, 1);
            } else {
                u8g2_DrawStr(&u8g2, 2, y, items[idx].c_str());
            }
        }

        display_update();
        delay_ms(50);
    }
}

bool display_prompt_yes_no(const std::string& title, const std::string& question) {
    std::vector<std::string> opts = {"Yes", "No"};
    return display_prompt_menu(question, opts) == 0;
}

void display_redraw_logs() {
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_profont10_tr); // A small, readable font

    for (size_t i = 0; i < log_buffer.size(); ++i) {
        u8g2_DrawStr(&u8g2, 0, (i + 1) * 9, log_buffer[i].c_str());
    }
    u8g2_SendBuffer(&u8g2);
}

void display_add_log_line(const std::string& log_line) {
    log_buffer.push_back(log_line);
    if (log_buffer.size() > MAX_LOG_LINES) {
        log_buffer.erase(log_buffer.begin());
    }
    // Always update the physical screen with the new log line
    display_redraw_logs();
}

bool is_oled_functional(){

    return oled_is_functional;
}