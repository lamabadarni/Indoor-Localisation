// display.h
#pragma once
#ifndef DISPLAY_H
#define DISPLAY_H

#include <vector>
#include <string>

// Initialize buttons and the OLED display; call once in app_main()
void display_init();

// Clear the in-RAM buffer (does not push to screen)
void display_clear();

// Flush the buffer to the screen
void display_update();

// Show a centered title + message for a fixed time (ms >= 0)
void display_show_message(const char* title, const char* message, int delay_ms);

// Show a scrollable menu. Returns selected index [0..items.size()-1], or -1 if empty.
int display_prompt_menu(const std::string& title, const std::vector<std::string>& items);

// Ask a yes/no question. Returns true for “Yes”, false for “No”
bool display_prompt_yes_no(const std::string& title, const std::string& question);
/**
 * @brief Redraws the screen with the current contents of the log buffer.
 * Call this to restore the default log view after an interaction.
 */
void display_redraw_logs();

/**
 * @brief Adds a line of text to the log buffer and updates the screen.
 * @param log_line The string to add to the log.
 */
void display_add_log_line(const std::string& log_line);

bool is_oled_functional();

char display_wait_for_any_button();

    
#endif // DISPLAY_H
