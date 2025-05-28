
#include "logger.h"
#include "../utils/platform.h"

void log_message(const char* level, const char* tag, const char* format, ...) {

    const char* color = "";
    
    if (strcmp(level, "INFO") == 0)
        color = "\\033[1;32m";  // Green
    else if (strcmp(level, "WARN") == 0)
        color = "\\033[1;33m";  // Yellow
    else if (strcmp(level, "ERROR") == 0)
        color = "\\033[1;31m";  // Red
    else if (strcmp(level, "DEBUG") == 0)
        color = "\\033[1;34m";  // Blue

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    unsigned long timestamp = millis_since_boot();
    printf("[%s][%s][%lums] %s\n", level, tag, timestamp, buffer);
}
