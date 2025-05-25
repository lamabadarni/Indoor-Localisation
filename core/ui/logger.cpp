#include "logger.h"
#include "platform.h"
#include <cstdarg>
#include <cstdio>

#ifdef ARDUINO
  #include <Arduino.h>
#endif

void log_message(const char* level, const char* tag, const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    unsigned long timestamp = millis_since_boot();

#ifdef ARDUINO
    Serial.print("[");
    Serial.print(level);
    Serial.print("][");
    Serial.print(tag);
    Serial.print("][");
    Serial.print(timestamp);
    Serial.print("ms] ");
    Serial.println(buffer);
#else
    printf("[%s][%s][%lums] %s\n", level, tag, timestamp, buffer);
#endif
}
