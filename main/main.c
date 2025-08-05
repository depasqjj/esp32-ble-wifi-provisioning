#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "app_wifi.h"

void app_main(void) {
    app_wifi_init();
    ESP_LOGI("MAIN", "Provisioning started or Wi-Fi started if already provisioned");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
