
#include "logger.h"
#include <cstdarg>
#include <cstdio>

void log_message(const char* level, const char* tag, const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    unsigned long timestamp = millis_since_boot();
    printf("[%s][%s][%lums] %s\n", level, tag, timestamp, buffer);
}
