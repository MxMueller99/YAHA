idf_component_register(SRCS "yaha.c"
                    INCLUDE_DIRS "."
                    REQUIRES wifi api time_manager esp_lcd esp_event nvs_flash esp_common esp_system esp_netif)
