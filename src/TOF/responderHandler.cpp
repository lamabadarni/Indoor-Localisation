#define RESPONDER_ID 1  // Change this before flashing each responder
/*
void initFTMResponder() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    char ssid[32];
    snprintf(ssid, sizeof(ssid), "ESP_TOF_RESPONDER_%d", RESPONDER_ID);

    wifi_config_t ap_config = {};
    strcpy((char*)ap_config.ap.ssid, ssid);
    ap_config.ap.ssid_len = strlen(ssid);
    ap_config.ap.channel = 6;
    ap_config.ap.authmode = WIFI_AUTH_OPEN;
    ap_config.ap.max_connection = 1;
    ap_config.ap.beacon_interval = 100;
    ap_config.ap.ftm_responder = true;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_ftm_resp_set_enabled(true));

    LOG_INFO("FTM_RESPONDER", "✅ Responder %d initialized (SSID: %s)", RESPONDER_ID, ssid);

    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    LOG_INFO("FTM_RESPONDER", "MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void collectResponderMACs() {
    LOG_INFO("MAC", "========== Responder MAC Collection ==========");

    // Check if already filled
    bool allSet = true;
    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        if (isMACUnset(SystemSetup::responderMacs[i])) {
            allSet = false;
            break;
        }
    }

    if (allSet) {
        LOG_INFO("MAC", "All responder MACs are already configured.");
        LOG_INFO("MAC", "Would you like to scan again anyway? (y/n)");
        char c = readCharFromUser();
        if (c != 'y' && c != 'Y') return;
    }

    delay_ms(USER_PROMPTION_DELAY);

    int found = verifyTOFScanCoverage();

    if (found == 0) {
        LOG_WARN("MAC", " No FTM-capable responders found.");
        LOG_INFO("MAC", "Please make sure responders are powered and within range.");
        return;
    }

    LOG_INFO("MAC", "Discovered %d responder(s).", found);
    for (int i = 0; i < NUMBER_OF_RESPONDERS; ++i) {
        if (!isMACUnset(SystemSetup::responderMacs[i])) {
            LOG_INFO("MAC", "Responder %d MAC: %s", i + 1, formatMac(SystemSetup::responderMacs[i]).c_str());
        }
    }

    LOG_INFO("MAC", "Would you like to save these MACs? (y/n)");
    char confirm = readCharFromUser();
    if (confirm != 'y' && confirm != 'Y') {
        LOG_WARN("MAC", "User chose not to save MAC addresses. Reverting.");
        clearResponderMacs(SystemSetup::responderMacs);
    } else {
        LOG_INFO("MAC", "Responder MACs saved.");
    }

    LOG_INFO("MAC", "==============================================");
}

*/
/* Wi-Fi FTM Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

///////////////////////////////////******/////////////////////// we added this !!! ////////////
/*
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"



void app_main(void)
{ 
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }


    // start console REPL
     wifi_config_t cfg = {
   .ap = {
    .ssid= "responder1",
    .ssid_len=0,
    .password="12345678",
    .channel = 1,
    .authmode = WIFI_AUTH_WPA2_PSK,
    .max_connection =4,
    .ftm_responder=true
    

   }

    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    for(;;){
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
*/