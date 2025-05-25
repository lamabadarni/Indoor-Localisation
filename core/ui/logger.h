#pragma once

void log_message(const char* level, const char* tag, const char* format, ...);

#define LOG_INFO(tag, ...)  log_message("INFO",  tag, __VA_ARGS__)
#define LOG_WARN(tag, ...)  log_message("WARN",  tag, __VA_ARGS__)
#define LOG_ERROR(tag, ...) log_message("ERROR", tag, __VA_ARGS__)
