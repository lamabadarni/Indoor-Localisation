// display.h
#pragma once
#ifndef DISPLAY_H
#define DISPLAY_H


#include "lvgl.h"          // Include LVGL main header
#include "esp_lvgl_port.h" // Include ESP-LVGL port header

#ifdef __cplusplus // <--- ONLY include these for C++ compilers
#include <vector>  // For std::vector
#include <string>  // For std::string
#endif // __cplusplus

#include "driver/gpio.h"   // For GPIO_NUM definitions

// --- Basic Hardware Definitions (from your working setup) ---
// Button pins


#ifdef __cplusplus
extern "C" { // <--- Start of extern "C" block for C functions
#endif

// Declare your C display initialization helper here
void display_init_helper(); // This function's definition is in a .c file

#ifdef __cplusplus
} // <--- End of extern "C" block
#endif
extern lv_disp_t *g_main_disp;



// --- Public Function Prototypes ---
void display_clear();
// display_update is usually not needed as LVGL handles flushing automatically

// Show a centered title + message for a fixed time (ms >= 0)

// Show a scrollable menu. Returns selected index [0..items.size()-1], or -1 if empty.
// This signature uses std::vector<std::string>

// Ask a yes/no question. Returns true for “Yes”, false for “No”

// Functions for managing an on-screen log buffer
// This signature uses std::string
void display_redraw_logs();


// Utility for checking display status
bool is_oled_functional();

// Function to wait for any button press (returns a char 'Y' for simplicity)
char display_wait_for_any_button();
#ifdef __cplusplus


// These functions use C++ types (std::string, std::vector) and are defined in src/core/ui/display.cpp (C++).
// They are only visible to other C++ files.
void display_show_message(const std::string& title, const std::string& message, int delay_time_ms);
int display_prompt_menu(const std::string& title, const std::vector<std::string>& items);
bool display_prompt_yes_no(const std::string& title, const std::string& question);
void display_add_log_line(const std::string& log_line); // This takes std::string

#endif // __cplusplus
// Global LVGL display handle declared here so main.cpp/userUIOled.cpp can access it
#endif // DISPLAY_H