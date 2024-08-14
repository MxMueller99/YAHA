idf_component_register(SRCS "api.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES freertos esp_http_client esp_system json)