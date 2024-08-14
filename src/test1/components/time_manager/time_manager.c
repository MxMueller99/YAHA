#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_sntp.h"


static const char *TAG = "time_module";

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Time synchronization event received");
}

void initialize_sntp(void) 
{
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();

    // Wait for time to be synchronized
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
        ESP_LOGI(TAG, "Waiting for time synchronization...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    ESP_LOGI(TAG, "Initialized SNTP");
    ESP_LOGI(TAG, "Time synchronized");

}

void obtain_time(char* buffer, size_t buffer_size) 
{
    ESP_LOGI(TAG, "Obtaining Current Time");
    time_t now;
    struct tm timeinfo;
    time(&now);
    setenv("TZ", "CET-1CEST", 1);
    tzset();

    localtime_r(&now, &timeinfo);
    strftime(buffer, buffer_size, "%c", &timeinfo);
    //ESP_LOGI(TAG, "The current date/time in Berlin is: %s", buffer);
}