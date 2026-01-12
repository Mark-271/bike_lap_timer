#ifndef ESP_NOW_PROTOCOL_H
#define ESP_NOW_PROTOCOL_H

#include <stdint.h>

typedef struct {
    uint32_t seq_num;
} esp_now_data_t;

#define ESP_NOW_CHANNEL 1
#define BROADCAST_MAC {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
#define ESP_NOW_PMK "pmk1234567890123"

#endif // ESP_NOW_PROTOCOL_H
