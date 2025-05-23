/*
Author: Max Müller

YAHA = Yet Another Home Assistant

*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "esp_log.h"
#include <string.h>
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lvgl_port.h"
#include "wifi.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "time_manager.h"
#include "api.h"

// Defines
#define TAG "YAHA"
#define HSPI_HOST SPI2_HOST
#define LCD_HOST    HSPI_HOST

// Define pin connections
/*
// ESP32
#define PIN_NUM_MOSI 23         // 11 for S3
#define PIN_NUM_CLK 18          // 12 for S3
#define PIN_NUM_CS 5            // 10 for S3
#define PIN_NUM_DC 16           // 9 for S3
#define PIN_NUM_RST 17          // 8 for S3
#define PIN_NUM_BCKL 4          // 7 for S3
*/
// ESP32S3
#define PIN_NUM_MOSI 11         // 11 for S3
#define PIN_NUM_CLK 12          // 12 for S3
#define PIN_NUM_CS 10            // 10 for S3
#define PIN_NUM_DC 9           // 9 for S3
#define PIN_NUM_RST 8          // 8 for S3
#define PIN_NUM_BCKL 7          // 7 for S3

// Define LCD
#define LCD_WIDTH 240
#define LCD_HEIGHT 280

// Define WIFI Events
#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1

// Handles
static esp_lcd_panel_io_handle_t io_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;
static spi_device_handle_t spi;

// LVGL Display
static lv_disp_t *lvgl_disp = NULL;

// LVGL Objects
static lv_obj_t *loadscreen_label = NULL;                  // Loadscreen
static lv_obj_t *load_bar = NULL;                          // Load Bar
static lv_obj_t *rectangle1 = NULL;                        // Rectangle 1 Background
static lv_obj_t *rectangle2 = NULL;                        // Rectangle 2 Background
static lv_obj_t *rectangle3 = NULL;                        // Rectangle 3 Background
static lv_obj_t *rectangle4 = NULL;                        // Rectangle 4 Background
static lv_obj_t *city_label = NULL;                        // City Text label
static lv_obj_t *temp_label = NULL;                        // Temperature label
static lv_obj_t *weather_label = NULL;                     // Weather type label


esp_err_t init_screen(void) {

    ESP_LOGI(TAG, "Starting LCD Initialization");

    // Initialize SPI bus 
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * 3, 
    };
    esp_err_t ret = spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }
    //ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));
    //ESP_ERROR_CHECK((HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Allocate an LCD IO device handle
    //esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_DC,
        .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = 18 * 1000 * 1000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t) LCD_HOST, &io_config, &io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LCD IO handle: %s", esp_err_to_name(ret));
        return ret;
    }
    //ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t) LCD_HOST, &io_config, &io_handle));

    // Initialize the LCD controller driver
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, // LCD_RGB_ELEMENT_ORDER_BGR or , or LCD_RGB_ELEMENT_ORDER_RGB
        .bits_per_pixel = 16,
    };
    ret = esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install LCD controller driver: %s", esp_err_to_name(ret));
        return ret;
    }
    //ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    // Initialize GPIO pins for control signals
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_BCKL, 1);

    // Initialize LCD Panel
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_set_gap(panel_handle, 0, 20); // Really important!
    esp_lcd_panel_init(panel_handle);
    
    // Debugging Color and gap error
    esp_lcd_panel_invert_color(panel_handle, true);
    

    //esp_lcd_panel_mirror(*panel_handle, true, false);  // Example for flipping/mirroring if needed
    //esp_lcd_panel_swap_xy(*panel_handle, true);        // Swap X and Y axes if needed for rotation
    //ESP_GOTO_ON_ERROR(esp_lcd_new_rgb_panel(&panel_config, &panel_handle), err, TAG, "RGB init failed"); // needed PERHAPS
    esp_lcd_panel_disp_on_off(panel_handle, true);
    
    ESP_LOGI(TAG, "LCD Initialization complete");

    return ESP_OK;
}

esp_err_t init_lvgl(void) {

    ESP_LOGI(TAG, "Starting LVGL Initialization");

    // Initialize LVGL
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,
        .task_stack = 4096,
        .task_affinity = 0,
        .task_max_sleep_ms = 500,
        .timer_period_ms = 5
    };
    esp_err_t ret = lvgl_port_init(&lvgl_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LVGL cfg: %s", esp_err_to_name(ret));
        return ret;
    }

    // Adding Screen
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = (LCD_WIDTH * LCD_HEIGHT) / 2,            // might need adjustment if memory is limited
        .double_buffer = true,                                  // set to false if memory is limited
        .hres = LCD_WIDTH,
        .vres = LCD_HEIGHT,
        .monochrome = false,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false
        },
        .flags = {
            .buff_dma = false
        }
    };
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);
    
    ESP_LOGI(TAG, "LVGL Initialization complete");

    return ESP_OK;
}

// Used to test the LCD screen
void fill_screen_with_color(esp_lcd_panel_handle_t panel_handle, uint16_t color) {
    // Create a buffer to hold the color data
    size_t buffer_size = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t);
    uint16_t *color_buffer = heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);
    if (color_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for color buffer");
        return;
    }

    // Fill Buffer with color
    for (int i=0; i < (LCD_WIDTH*LCD_HEIGHT); i++) {
        color_buffer[i] = color;
    }

    // Draw the buffer on the screen
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, LCD_WIDTH, LCD_HEIGHT, color_buffer);

    // Free buffer
    heap_caps_free(color_buffer);
}

// Used to test the LCD screen
void color_switch_task(void *arg) {

    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) arg;

    while (1) {
        // Fill screen red
        fill_screen_with_color(panel_handle, 0xF800);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Fill screen white
        fill_screen_with_color(panel_handle, 0xFFFF);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Fill screen black
        fill_screen_with_color(panel_handle, 0x0000);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Fill screen random color
        fill_screen_with_color(panel_handle, 0xAFB0);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Fill screen random color
        fill_screen_with_color(panel_handle, 0xFBFB);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Fill screen white
        fill_screen_with_color(panel_handle, 0xFFFF);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// Loadscreen display
void display_loadscreen(const char *text) {
    // Create Label if NULL
    if (loadscreen_label == NULL) {
        // Label style
        static lv_style_t text_style;
        // Set Font and color
        lv_style_set_text_font(&text_style, &lv_font_montserrat_30); 
        lv_style_set_text_color(&text_style, lv_color_hex(0x69E732));
        // Label creation
        loadscreen_label = lv_label_create(lv_scr_act());  // Create a label on the active screen
        // Apply the style to the label
        lv_obj_add_style(loadscreen_label, &text_style, 0);
        // Align the label to the center of the display
        lv_obj_align(loadscreen_label, LV_ALIGN_CENTER, 0, 0);
    }

    // Update Text
    lv_label_set_text(loadscreen_label, text);                   // Set the text for the label
}

// Clears the loadscreen
void clear_loadscreen() {
    if (loadscreen_label != NULL) {
        lv_obj_del(loadscreen_label);
        loadscreen_label = NULL;
    }
}

// Initializes a Load Bar and updates the progress bar
void display_loadbar(int progress) {
    // Load Bar creation if NULL
    if (load_bar == NULL) {
        load_bar = lv_bar_create(lv_scr_act());
        lv_obj_set_size(load_bar, 160, 20);
        lv_obj_align(load_bar, LV_ALIGN_CENTER, 0, 40);        // Align the bar to the center - 40 y offset
        lv_obj_set_style_bg_color(load_bar, lv_color_hex(0xFFFFFF), LV_PART_MAIN);  // Style the bar Background
        lv_obj_set_style_bg_opa(load_bar, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(load_bar, lv_color_hex(0x00FF00), LV_PART_INDICATOR); // Style the Indicator
        lv_obj_set_style_bg_opa(load_bar, LV_OPA_COVER, LV_PART_INDICATOR);

    }

    lv_bar_set_value(load_bar, progress, LV_ANIM_ON);
}

// Clears the Load Bar
void clear_loadbar() {
    if (load_bar != NULL) {
        lv_obj_del(load_bar);
        load_bar = NULL;
    }
}

// Display Weather Background
void display_weather_background() {
    // Colors
    lv_color_t color4 = lv_color_hex(0xFFFFFF); // Lightest color
    lv_color_t color3 = lv_color_hex(0xFFD0F0);
    lv_color_t color2 = lv_color_hex(0xFFC1F3);
    lv_color_t color1 = lv_color_hex(0xFFA0A0); // Darkest color

    // Draw circles in reverse order
    rectangle4 = lv_obj_create(lv_scr_act());
    lv_obj_set_size(rectangle4, 240, 70);
    lv_obj_set_style_bg_color(rectangle4, color4, LV_PART_MAIN);
    lv_obj_set_style_border_width(rectangle4, 0, LV_PART_MAIN);
    lv_obj_align(rectangle4, LV_ALIGN_TOP_MID, 0, 5);
    
    rectangle3 = lv_obj_create(lv_scr_act());
    lv_obj_set_size(rectangle3, 240, 70);
    lv_obj_set_style_bg_color(rectangle3, color3, LV_PART_MAIN);
    lv_obj_set_style_border_width(rectangle3, 0, LV_PART_MAIN);
    lv_obj_align(rectangle3, LV_ALIGN_TOP_MID, 0, 75);
    
    rectangle2 = lv_obj_create(lv_scr_act());
    lv_obj_set_size(rectangle2, 240, 70);
    lv_obj_set_style_bg_color(rectangle2, color2, LV_PART_MAIN);
    lv_obj_set_style_border_width(rectangle2, 0, LV_PART_MAIN);
    lv_obj_align(rectangle2, LV_ALIGN_TOP_MID, 0, 145);
    
    rectangle1 = lv_obj_create(lv_scr_act());
    //lv_obj_set_style_radius(circle1, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_size(rectangle1, 240, 70);
    lv_obj_set_style_bg_color(rectangle1, color1, LV_PART_MAIN);
    lv_obj_set_style_border_width(rectangle1, 0, LV_PART_MAIN);
    lv_obj_align(rectangle1, LV_ALIGN_TOP_MID, 0, 210);
    
}

// Clears Weather Background
void clear_background() {
    
    if (rectangle1 != NULL) {
        lv_obj_del(rectangle1);
        rectangle1 = NULL;
    }
    if (rectangle2 != NULL) {
        lv_obj_del(rectangle2);
        rectangle2 = NULL;
    }
    
    if (rectangle3 != NULL) {
        lv_obj_del(rectangle3);
        rectangle3 = NULL;
    }
    if (rectangle4 != NULL) {
        lv_obj_del(rectangle4);
        rectangle4 = NULL;
    }
}

void draw_weather(char temp_buffer[20], char weather_type_buffer[30]) {
    // Label style for both
    static lv_style_t text_style;
    static lv_style_t type_style;

    lv_style_set_text_font(&text_style, &lv_font_montserrat_30); 
    lv_style_set_text_color(&text_style, lv_color_hex(0x000000));

    lv_style_set_text_font(&type_style, &lv_font_montserrat_14);
    lv_style_set_text_color(&type_style, lv_color_hex(0x000000));

    // Label creation
    city_label = lv_label_create(lv_scr_act());
    temp_label = lv_label_create(lv_scr_act());
    weather_label = lv_label_create(lv_scr_act());
    // Apply the style to the label
    lv_obj_add_style(city_label, &text_style, 0);
    lv_obj_add_style(temp_label, &text_style, 0);
    lv_obj_add_style(weather_label, &type_style, 0);
    // Align the label to the center of the display
    lv_obj_align(city_label, LV_ALIGN_CENTER, 0, 20);
    lv_obj_align(temp_label, LV_ALIGN_CENTER, 0, -20);
    lv_obj_align(weather_label, LV_ALIGN_CENTER, 0, 0);

    lv_label_set_text(city_label, CONFIG_OPENWEATHER_CITY);
    lv_label_set_text(temp_label, temp_buffer);
    lv_label_set_text(weather_label, weather_type_buffer);
}

void update_weather(void *arg) {

    char weather_buffer[1024];
    char temp_buffer[20];
    char main_buffer[30];

    while(1) {
        get_current_weather_data(weather_buffer, sizeof(weather_buffer), temp_buffer, sizeof(temp_buffer), main_buffer, sizeof(main_buffer));

        lvgl_port_lock(0);
        lv_label_set_text(temp_label, temp_buffer);
        lv_label_set_text(weather_label, main_buffer);
        lvgl_port_unlock();

        // Wait 5 minutes
        vTaskDelay(pdMS_TO_TICKS(5 * 60 * 1000));
    }

   
}

void clear_weather() {
    if (city_label != NULL) {
        lv_obj_del(city_label);
        city_label = NULL;
    }

    if (temp_label != NULL) {
        lv_obj_del(temp_label);
        temp_label = NULL;
    }

    if (weather_label != NULL) {
        lv_obj_del(weather_label);
        weather_label = NULL;
    }   
}

// Main task for the application
void app_main() {

    /* Screen Initialization START */
    // LCD Init
    esp_err_t ret = init_screen();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Screen initialization failed");
        return;
    }
    // LVGL Init    
    ret = init_lvgl();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LVGL initialization failed");
        return;
    }
    /* Screen Initialization END */

    /* Loadup Bar Initialization START */
    // Initial Loadup display
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), LV_PART_MAIN);
    display_loadscreen("YAHA!");
    display_loadbar(0);
    vTaskDelay(pdMS_TO_TICKS(3000));
    /* Loadup Bar Initialization END */

    /* WIFI Initialization START */
    // Update Homescreen
    display_loadscreen("WiFi Init");
    display_loadbar(10);
    vTaskDelay(pdMS_TO_TICKS(2000));
    

    // Initialize Non-Volatile Storage
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    display_loadbar(20);
    // Initialize WiFi in station mode and connect
    esp_err_t wifi_status = WIFI_FAILURE;
    wifi_status = wifi_init_sta();
    if (WIFI_SUCCESS != wifi_status)
	{
		ESP_LOGI(TAG, "Failed to associate to AP");
		return;
	}

    vTaskDelay(pdMS_TO_TICKS(1000));
    display_loadbar(30);

    // WiFi Delay, remove if not needed
    vTaskDelay(pdMS_TO_TICKS(2000));
    // WIFI Initialization END

    // Time module Initilization Start
    // Update Homescreen
    display_loadscreen("SNTP Init");
    vTaskDelay(pdMS_TO_TICKS(500));
    display_loadbar(35);
    
    // Initialize time
    initialize_sntp();
    vTaskDelay(pdMS_TO_TICKS(500));
    display_loadbar(40);
    // Buffer to hold current time
    char time_buffer[64]; // Holds the updated time
    // Obtain current time and save in buffer
    obtain_time(time_buffer, sizeof(time_buffer));

    vTaskDelay(pdMS_TO_TICKS(1000));
    display_loadbar(50);
    // SNTP Delay, remove if not needed
    vTaskDelay(pdMS_TO_TICKS(2000));
    // Time module Initilization END

    // HTTP Request
    display_loadscreen("HTTP Request");
    vTaskDelay(pdMS_TO_TICKS(500));
    display_loadbar(60);
    char weather_buffer[1024];
    char temp_buffer[20];
    char main_buffer[30];
    get_current_weather_data(weather_buffer, sizeof(weather_buffer), temp_buffer, sizeof(temp_buffer), main_buffer, sizeof(main_buffer));
    vTaskDelay(pdMS_TO_TICKS(500));
    display_loadbar(70);
    // perform weather forecast http request
    vTaskDelay(pdMS_TO_TICKS(1000));
    display_loadbar(80);

    
    // Clear Init Screen
    clear_loadbar();
    clear_loadscreen();

    display_weather_background();

    // render weather screen in lvgl
    draw_weather(temp_buffer, main_buffer);

    // keep updating weather every 5 minutes
    xTaskCreatePinnedToCore(update_weather, "update_weather_task", 4096, NULL, 5, NULL, 1); 
}
