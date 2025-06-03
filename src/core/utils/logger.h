#pragma once

#define ANSI_RESET  "\\033[0m"
#define ANSI_GREEN  "\\033[1;32m"
#define ANSI_YELLOW "\\033[1;33m"
#define ANSI_RED    "\\033[1;31m"
#define ANSI_BLUE   "\\033[1;34m"

void log_message(const char* level, const char* tag, const char* format, ...);

#define LOG_ERROR(tag, ...) if (SystemSetup::logLevel >= LOG_LEVEL_ERROR)  log_message("ERROR", tag, __VA_ARGS__)
#define LOG_WARN(tag, ...)  if (SystemSetup::logLevel >= LOG_LEVEL_WARN)   log_message("WARN", tag, __VA_ARGS__)
#define LOG_INFO(tag, ...)  if (SystemSetup::logLevel >= LOG_LEVEL_INFO)   log_message("INFO", tag, __VA_ARGS__)
#define LOG_DEBUG(tag, ...) if (SystemSetup::logLevel >= LOG_LEVEL_DEBUG)  log_message("DEBUG", tag, __VA_ARGS__)
