#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_now_protocol.h"
#include "debug_utils.h"

static const char *TAG = "RECEIVER";
static TimerHandle_t timeout_timer;
static bool connection_established = false;

#ifndef CONFIG_RSSI_THRESHOLD_HIGH
#define CONFIG_RSSI_THRESHOLD_HIGH -30
#endif

#ifndef CONFIG_RSSI_THRESHOLD_LOW
#define CONFIG_RSSI_THRESHOLD_LOW -40
#endif

typedef enum {
    LAP_STATE_IDLE,
    LAP_STATE_DETECTED
} lap_state_t;

static lap_state_t lap_state = LAP_STATE_IDLE;
static QueueHandle_t rssi_queue;

static void timeout_callback(TimerHandle_t xTimer) {
    ESP_LOGW(TAG, "WARNING: No packets received for >500ms - transmitter out of range");
    connection_established = false;
}

static void process_rssi_hysteresis(int8_t rssi) {
    switch (lap_state) {
        case LAP_STATE_IDLE:
            if (rssi > CONFIG_RSSI_THRESHOLD_HIGH) {
                lap_state = LAP_STATE_DETECTED;
                ESP_LOGI(TAG, "LAP DETECTED! RSSI: %d dBm", rssi);
                // TODO: Record lap time, trigger actions
            }
            break;
            
        case LAP_STATE_DETECTED:
            if (rssi < CONFIG_RSSI_THRESHOLD_LOW) {
                lap_state = LAP_STATE_IDLE;
                ESP_LOGI(TAG, "Lap complete, back to idle");
            }
            break;
    }
}

static void lap_detection_task(void *pvParameter) {
    int8_t rssi;
    while (1) {
        if (xQueueReceive(rssi_queue, &rssi, portMAX_DELAY)) {
            process_rssi_hysteresis(rssi);
        }
    }
}

static void esp_now_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (len == sizeof(esp_now_data_t)) {
        esp_now_data_t *received_data = (esp_now_data_t *)data;
        
        if (!connection_established) {
            ESP_LOGI(TAG, "Connection established with transmitter");
            connection_established = true;
        }
        
        ESP_LOGI(TAG, "Received sequence number: %lu, RSSI: %d dBm", 
                 received_data->seq_num, recv_info->rx_ctrl->rssi);
        
        xQueueSend(rssi_queue, &recv_info->rx_ctrl->rssi, 0);
        
        // Reset timeout timer
        xTimerReset(timeout_timer, 0);
    }
}

void app_main(void) {
    debug_init();
    
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
    
    rssi_queue = xQueueCreate(1, sizeof(int8_t));
    xTaskCreate(lap_detection_task, "lap_detection", 4096, NULL, 5, NULL);
    
    timeout_timer = xTimerCreate("timeout_timer", pdMS_TO_TICKS(500), pdFALSE, NULL, timeout_callback);
    
    ESP_LOGI(TAG, "Receiver started - waiting for transmitter packets");
    
    debug_run_monitoring_loop();
}
