#include "responderHandler.h"

static const char *TAG = "FTM_RESPONDER";

void initFTMResponder(void)
{
    LOG_DEBUG(TAG, "Initializing NVS...");
    ESP_ERROR_CHECK(nvs_flash_init());

    LOG_DEBUG(TAG, "Initializing netif...");
    ESP_ERROR_CHECK(esp_netif_init());

    // DO NOT create event loop again if your system already did.
    // If not yet created, uncomment below:
    // LOG_DEBUG(TAG, "Creating default event loop...");
    // ESP_ERROR_CHECK(esp_event_loop_create_default());

    LOG_DEBUG(TAG, "Creating default WiFi AP interface...");
    esp_netif_create_default_wifi_ap();

    LOG_DEBUG(TAG, "Initializing WiFi...");
    wifi_init_config_t cfg_init = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg_init));

    wifi_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    LOG_DEBUG(TAG, "Configuring AP...");
    strcpy((char *)cfg.ap.ssid, "MyAP");
    cfg.ap.ssid_len = strlen("MyAP");
    cfg.ap.channel = 1;
    cfg.ap.authmode = WIFI_AUTH_OPEN;
    cfg.ap.ssid_hidden = 0;
    cfg.ap.max_connection = 1;
    cfg.ap.beacon_interval = 100;
    cfg.ap.ftm_responder = true;
    cfg.ap.pmf_cfg.capable = true;
    cfg.ap.pmf_cfg.required = false;
    cfg.ap.pairwise_cipher = WIFI_CIPHER_TYPE_NONE;
    cfg.ap.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
    cfg.ap.transition_disable = false;

    LOG_DEBUG(TAG, "Setting WiFi mode to AP...");
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    LOG_DEBUG(TAG, "Applying WiFi config...");
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &cfg));

    LOG_DEBUG(TAG, "Starting WiFi...");
    ESP_ERROR_CHECK(esp_wifi_start());

    LOG_DEBUG(TAG, "Responder ready and running.");

    ESP_ERROR_CHECK(esp_read_mac(responderMacs[0], ESP_MAC_WIFI_SOFTAP));
    
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
         responderMacs[0][0], responderMacs[0][1], responderMacs[0][2], responderMacs[0][3], responderMacs[0][4], responderMacs[0][5]);
    LOG_DEBUG(TAG, "Device MAC (SoftAP): %s", macStr);
    return;
}