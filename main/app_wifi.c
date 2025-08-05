#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"
#include "app_wifi.h"

#define TAG "app_wifi"

event_handler_instance_t instance_any_id;
event_handler_instance_t instance_got_ip;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Got IP address");
    }
}

void app_wifi_init(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
    };

    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    bool provisioned = false;
    wifi_prov_mgr_is_provisioned(&provisioned);

    if (!provisioned) {
        ESP_LOGI(TAG, "Starting provisioning");

        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
        const char *pop = "abcd1234";

        wifi_prov_mgr_start_provisioning(security, pop, NULL);
    } else {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi");
        wifi_config_t wifi_cfg;
        ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_cfg));
        ESP_LOGI(TAG, "Connecting to SSID: %s", wifi_cfg.sta.ssid);

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }
}