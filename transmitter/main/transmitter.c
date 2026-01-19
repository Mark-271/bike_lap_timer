#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_now_protocol.h"

static const char *TAG = "TRANSMITTER";
static uint32_t sequence_number = 0;
static uint8_t broadcast_mac[6] = BROADCAST_MAC;

#define WIFI_TX_POWER_DBM 5
#define DBM_TO_POWER_UNITS(dbm) ((dbm) * 4)

static void esp_now_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        ESP_LOGI(TAG, "Packet %lu sent successfully", sequence_number);
    } else {
        ESP_LOGW(TAG, "Packet %lu send failed", sequence_number);
    }
}

static void set_wifi_tx_power(int8_t power) {
    esp_wifi_set_max_tx_power(power);
    int8_t actual_power;
    esp_wifi_get_max_tx_power(&actual_power);
    ESP_LOGI(TAG, "TX power set to %d (0.25 dBm units), actual: %d (%.2f dBm)", 
             power, actual_power, actual_power * 0.25);
}

static void transmitter_task(void *pvParameter) {
    esp_now_data_t data;
    
    while (1) {
        data.seq_num = ++sequence_number;
        
        esp_err_t result = esp_now_send(broadcast_mac, (uint8_t *)&data, sizeof(data));
        if (result != ESP_OK) {
            ESP_LOGE(TAG, "Send error: %s", esp_err_to_name(result));
        }
        
        ESP_LOGI(TAG, "Send msg: %u %s", data.seq_num, esp_err_to_name(result));
        vTaskDelay(pdMS_TO_TICKS(500));
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
    
    set_wifi_tx_power(DBM_TO_POWER_UNITS(WIFI_TX_POWER_DBM));
    
    ESP_ERROR_CHECK(esp_now_init());
    // ESP_ERROR_CHECK(esp_now_register_send_cb(esp_now_send_cb));
    ESP_ERROR_CHECK(esp_now_set_pmk((uint8_t *)ESP_NOW_PMK));
    
    esp_now_peer_info_t peer;
    memset(&peer, 0, sizeof(esp_now_peer_info_t));
    peer.channel = ESP_NOW_CHANNEL;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;
    memcpy(peer.peer_addr, broadcast_mac, 6);
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));
    
    ESP_LOGI(TAG, "Transmitter started - broadcasting every 100ms");
    xTaskCreate(transmitter_task, "transmitter_task", 4096, NULL, 5, NULL);
}
