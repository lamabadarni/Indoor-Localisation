#include <WiFi.h>

const char *ssid = "Balcony";
const char *password = "noPassword";

void setup(void)
{
  // set ESP32 WiFi mode
  WiFi.mode(WIFI_AP);
  // Start the AP with ssid and pasword
  WiFi.softAP(ssid, password);
}

void loop(void) {}