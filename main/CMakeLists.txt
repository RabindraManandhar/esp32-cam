idf_component_register(SRCS "main.c"
                    PRIV_REQUIRES esp32-camera esp_wifi esp_http_client nvs_flash
                    INCLUDE_DIRS ".")
