#include "debug_utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "DEBUG";

#ifdef CONFIG_STACK_DEBUG

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    printf("STACK OVERFLOW in task: %s\n", pcTaskName);
    UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(xTask);
    printf("Stack high water mark: %lu bytes remaining\n", uxHighWaterMark * sizeof(StackType_t));
    esp_restart();
}

static void debug_monitor_stack(void) {
    // Get number of tasks
    UBaseType_t task_count = uxTaskGetNumberOfTasks();
    TaskStatus_t *task_array = malloc(task_count * sizeof(TaskStatus_t));
    
    if (task_array != NULL) {
        // Get all task info
        UBaseType_t actual_count = uxTaskGetSystemState(task_array, task_count, NULL);
        
        ESP_LOGI(TAG, "=== Stack Usage Report ===");
        for (UBaseType_t i = 0; i < actual_count; i++) {
            UBaseType_t stack_remaining = uxTaskGetStackHighWaterMark(task_array[i].xHandle);
            UBaseType_t stack_used = task_array[i].usStackHighWaterMark;
            
            ESP_LOGI(TAG, "Task: %-12s Stack: %4lu/%4lu bytes (%lu%% used)", 
                     task_array[i].pcTaskName,
                     stack_used * sizeof(StackType_t),
                     (stack_used + stack_remaining) * sizeof(StackType_t),
                     (stack_used * 100) / (stack_used + stack_remaining));
        }
        
        free(task_array);
    } else {
        ESP_LOGW(TAG, "Failed to allocate memory for task monitoring");
    }
}

void debug_init(void) {
    ESP_LOGI(TAG, "Stack debugging enabled");
}

void debug_run_monitoring_loop(void) {
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        
        debug_monitor_stack();
        
        // Add more debug monitors here:
        // debug_monitor_heap();
        // debug_monitor_wifi();
        // debug_monitor_performance();
    }
}

#else

void debug_init(void) {}
void debug_run_monitoring_loop(void) {
    // No-op: function returns immediately, main task exits normally
}

#endif
