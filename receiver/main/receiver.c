#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_now_protocol.h"

static const char *TAG = "RECEIVER";
static TimerHandle_t timeout_timer;
static bool connection_established = false;

static void timeout_callback(TimerHandle_t xTimer) {
    ESP_LOGW(TAG, "WARNING: No packets received for >500ms - transmitter out of range");
    connection_established = false;
}

static void esp_now_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (len == sizeof(esp_now_data_t)) {
        esp_now_data_t *received_data = (esp_now_data_t *)data;
        
        if (!connection_established) {
            ESP_LOGI(TAG, "Connection established with transmitter");
            connection_established = true;
        }
        
        ESP_LOGI(TAG, "Received sequence number: %lu", received_data->seq_num);
        
        // Reset timeout timer
        xTimerReset(timeout_timer, 0);
    }
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(ESP_NOW_CHANNEL, WIFI_SECOND_CHAN_NONE));
    
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(esp_now_recv_cb));
    ESP_ERROR_CHECK(esp_now_set_pmk((uint8_t *)ESP_NOW_PMK));
    
    // Create timeout timer (500ms)
    timeout_timer = xTimerCreate("timeout_timer", pdMS_TO_TICKS(500), pdFALSE, NULL, timeout_callback);
    
    ESP_LOGI(TAG, "Receiver started - waiting for transmitter packets");
}
