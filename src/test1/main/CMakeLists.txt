idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES wifi api time_manager esp_netif freertos esp_system esp_event nvs_flash esp_common)
