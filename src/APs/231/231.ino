#include <WiFi.h>

const char *ssid = "231";
const char *password = "noPassword";

void setup(void)
{
  // set ESP32 WiFi mode
  WiFi.mode(WIFI_AP);
  // Start the AP with ssid and pasword
  WiFi.softAP(ssid, password);
  // delay wait for AP setup
}

void loop(void) {}