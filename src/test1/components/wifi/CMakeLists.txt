idf_component_register(SRCS "wifi.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES freertos esp_system esp_wifi esp_event nvs_flash esp_common)