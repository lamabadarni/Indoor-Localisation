
#include "logger.h"
#include "../utils/platform.h"
#include "../ui/display.h"

void log_message_serial(const char* level, const char* tag, const char* format, ...) {

    const char* color = "";
    
    if (strcmp(level, "INFO") == 0)
        color = "\033[1;32m";  // Green
    else if (strcmp(level, "ERROR") == 0)
        color = "\033[1;31m";  // Red
    else if (strcmp(level, "DEBUG") == 0)
        color = "\033[1;34m";  // Blue

    if (strcmp(tag, "USER") == 0) {
        color = "\033[1;35m";
    }

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    unsigned long timestamp = millis_since_boot();
    printf("%s[%s][%s][%lums] %s\033[0m\n", color, level, tag, timestamp, buffer);
    fflush(stdout);                        
    vTaskDelay(pdMS_TO_TICKS(100));          
    if (logFile) {
        fprintf(logFile, "[%s][%s][%lums] %s\n", level, tag, timestamp, buffer);
        fflush(logFile);
    }
}

void log_message_oled(const char* level, const char* tag, const char* format, ...) {
    
    // Format the core message from the variable arguments
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Create the full log string to be displayed
    // NOTE: Timestamp is removed to save screen space, but you can add it back if you wish.
    char full_log_cstr[300];
    snprintf(full_log_cstr, sizeof(full_log_cstr), "[%s] %s", tag, buffer);
    
    // Send the complete, formatted string to our new OLED log viewer
    // display_add_log_line(std::string(full_log_cstr));

    if (logFile) {
        fprintf(logFile, full_log_cstr);
    }

    // The original printf to the serial terminal is now removed.
    // unsigned long timestamp = millis_since_boot();
    // printf("[%s][%s][%lums] %s\n", level, tag, timestamp, buffer);
}