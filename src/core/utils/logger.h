#pragma once

#include "utilities.h"

#define ANSI_RESET  "\\033[0m"
#define ANSI_GREEN  "\\033[1;32m"
#define ANSI_YELLOW "\\033[1;33m"
#define ANSI_RED    "\\033[1;31m"
#define ANSI_BLUE   "\\033[1;34m"

void log_message_serial(const char* level, const char* tag, const char* format, ...);
void log_message_oled(const char* level, const char* tag, const char* format, ...);

#define LOG_ERROR(tag, ...) if(systemUI == SERIAL && SystemSetup::logLevel >= LOG_LEVEL_ERROR) \ 
                                log_message_serial("ERROR", tag, __VA_ARGS__); \ 
                            else if(systemUI == OLED && SystemSetup::logLevel >= LOG_LEVEL_ERROR) \
                                log_message_oled("ERROR", tag, __VA_ARGS__);
#define LOG_INFO(tag, ...)  if(systemUI == SERIAL && SystemSetup::logLevel >= LOG_LEVEL_INFO) \
                                log_message_serial("INFO", tag, __VA_ARGS__); \
                            else if(systemUI == OLED && SystemSetup::logLevel >= LOG_LEVEL_INFO) \
                                log_message_oled("INFO", tag, __VA_ARGS__);
#define LOG_DEBUG(tag, ...) if(systemUI == SERIAL && SystemSetup::logLevel >= LOG_LEVEL_DEBUG) \
                                log_message_serial("DEBUG", tag, __VA_ARGS__); \
                            else if(systemUI == OLED && SystemSetup::logLevel >= LOG_LEVEL_DEBUG) \
                                log_message_oled("DEBUG", tag, __VA_ARGS__);
