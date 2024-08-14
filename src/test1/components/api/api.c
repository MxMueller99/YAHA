#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "api.h"
#include "esp_system.h"
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <ctype.h>
#include <cJSON.h>

// Definitions
static const char *TAG = "api_module";
#define MAX_URL_LENGTH 256

// API Configs
#define OPENWEATHER_API_KEY CONFIG_OPENWEATHER_API_KEY
#define OPENWEATHER_CITY CONFIG_OPENWEATHER_CITY

typedef struct {
    char *buffer;
    int buffer_size;
    int data_written;
} http_buffer_t;

// Event handler
esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{

    http_buffer_t *http_buf = (http_buffer_t *)evt->user_data;

    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // Ensure no overflow happens
                //printf("%.*s", evt->data_len, (char*)evt->data);
                if (http_buf->data_written + evt->data_len < http_buf->buffer_size)
                {
                    memcpy(http_buf->buffer + http_buf->data_written, evt->data, evt->data_len);
                    http_buf->data_written += evt->data_len;
                } else {
                    ESP_LOGW(TAG, "Buffer overflow prevented, data is too large");
                }
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
            break;
    }
    return ESP_OK;
}

void get_weather_data(char *buffer, int buffer_size)
{

    http_buffer_t http_weather_buffer = {
        .buffer = buffer,
        .buffer_size = buffer_size,
        .data_written = 0
    };

    char openweather_url[MAX_URL_LENGTH];
    snprintf(openweather_url, MAX_URL_LENGTH, "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s", OPENWEATHER_CITY,OPENWEATHER_API_KEY);

    esp_http_client_config_t config = {
        .url = openweather_url,
        .event_handler = _http_event_handle,
        .user_data = &http_weather_buffer, 
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Perform the HTTP GET request
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Status = %d, content_length = %lld",
           esp_http_client_get_status_code(client),
           esp_http_client_get_content_length(client));
        
        // Null Termination
        if (http_weather_buffer.data_written < buffer_size)
        {
            buffer[http_weather_buffer.data_written] = '\0';
        } else {
            buffer[buffer_size - 1] = '\0';
        }

        // Json Parsing
        cJSON *json = cJSON_Parse(buffer);
        if (json == NULL) {
            ESP_LOGE(TAG, "Failed to parse weather data");
        } else {
            cJSON *main = cJSON_GetObjectItem(json, "main");
            if (main != NULL) {
                cJSON *temp = cJSON_GetObjectItem(main, "temp");
                if (cJSON_IsNumber(temp)) {
                    // Convert temperature from Kelvin to Celsius
                    double temperature_celsius = temp->valuedouble - 273.15;

                    // Format the temperature into the buffer
                    snprintf(buffer, buffer_size, "%.2f°C", temperature_celsius);
                } else {
                    ESP_LOGE(TAG, "Temperature not found in JSON");
                }
            } else {
                ESP_LOGE(TAG, "Main section not found in JSON");
            }
            cJSON_Delete(json);
        }

    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}