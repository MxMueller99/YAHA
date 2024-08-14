#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "wifi.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_event.h"
#include "api.h"
#include "time_manager.h"

// Logger Tag
static const char *TAG = "main";

// Defines
#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1

void app_main(void)
{
    // Initialize Non-Volatile Storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize WiFi in station mode and connect
    esp_err_t wifi_status = WIFI_FAILURE;
    wifi_status = wifi_init_sta();
    if (WIFI_SUCCESS != wifi_status)
	{
		ESP_LOGI(TAG, "Failed to associate to AP, dying...");
		return;
	} 
    
    // WiFi Delay
    // ToDo: Remove if not needed
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Initialize time
    initialize_sntp();

    // Buffer to hold current time
    char time_buffer[64]; // Holds the updated time

    // Obtain current time and save in buffer
    obtain_time(time_buffer, sizeof(time_buffer));

    // Print time
    /*
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        obtain_time(time_buffer, sizeof(time_buffer));
        ESP_LOGI(TAG, "The Time is %s", time_buffer);
    }
    */    

    // DEBUG HTTP API Call
    // Fetch and print simple data
    ESP_LOGI(TAG, "Fetching simple data...");

    // Buffer to hold current weather
    char weather_buffer[1024];

    // Fetch and print weather data
    ESP_LOGI(TAG, "Fetching weather data...");
    get_weather_data(weather_buffer, sizeof(weather_buffer));
    ESP_LOGI(TAG, "The Temperature is %s", weather_buffer);
}
