#include <esp_system.h>
#include <nvs_flash.h>

#include "esp_camera.h"
#include "esp_log.h"
#include "esp_event.h"

#include "esp_wifi.h"
#include "esp_http_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Wi-Fi Configuration
#define WIFI_SSID "<WIFI_SSID>"
#define WIFI_PASS "<WIFI_PASSWORD>"

// Server URL
#define SERVER_URL "SERVER_URL/upload"

// support IDF 5.x
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

// OV2640 Camera Pin Configuration for ESP32-CAM (AI-Thinker)
#define CAM_PIN_PWDN     32
#define CAM_PIN_RESET    -1  // Software reset
#define CAM_PIN_XCLK     0
#define CAM_PIN_SIOD     26  // I2C SDA
#define CAM_PIN_SIOC     27  // I2C SCL
#define CAM_PIN_D7       35
#define CAM_PIN_D6       34
#define CAM_PIN_D5       39
#define CAM_PIN_D4       36
#define CAM_PIN_D3       21
#define CAM_PIN_D2       19
#define CAM_PIN_D1       18
#define CAM_PIN_D0       5
#define CAM_PIN_VSYNC    25
#define CAM_PIN_HREF     23
#define CAM_PIN_PCLK     22

static const char *TAG = "ESP32-CAM";

camera_config_t camera_config = {
  .pin_pwdn = CAM_PIN_PWDN,
  .pin_reset = CAM_PIN_RESET,
  .pin_xclk = CAM_PIN_XCLK,
  .pin_sscb_sda = CAM_PIN_SIOD,
  .pin_sscb_scl = CAM_PIN_SIOC,
  .pin_d7 = CAM_PIN_D7,
  .pin_d6 = CAM_PIN_D6,
  .pin_d5 = CAM_PIN_D5,
  .pin_d4 = CAM_PIN_D4,
  .pin_d3 = CAM_PIN_D3,
  .pin_d2 = CAM_PIN_D2,
  .pin_d1 = CAM_PIN_D1,
  .pin_d0 = CAM_PIN_D0,
  .pin_vsync = CAM_PIN_VSYNC,
  .pin_href = CAM_PIN_HREF,
  .pin_pclk = CAM_PIN_PCLK,
  .xclk_freq_hz = 20000000,    // 20MHz XCLK (required for OV2640)
  .ledc_timer = LEDC_TIMER_0,
  .ledc_channel = LEDC_CHANNEL_0,
  .pixel_format = PIXFORMAT_JPEG, // Use JPEG for OV2640
  .frame_size = FRAMESIZE_VGA,   // Resolution (e.g., FRAMESIZE_UXGA, FRAMESIZE_QVGA)
  .jpeg_quality = 12,             // Quality (0-63, lower = better)
  .fb_count = 1,                   // Frame buffers (use 2 for better performance)
  .fb_location = CAMERA_FB_IN_PSRAM,
  .grab_mode = CAMERA_GRAB_WHEN_EMPTY,    //CAMERA_GRAB_LATEST. Sets when buffers should be filled
};

// Wi-Fi Event Handler
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi disconnected, reconnecting...");
        esp_wifi_connect();
    }
}

// Initialize Wi-Fi
void init_wifi(void){
    // Initialize TCP/IP stack and default event loop
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    // Configure Wi-Fi in station mode
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);


    // Set Wi-Fi mode and credentials
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
 
    ESP_LOGI(TAG, "Connecting to Wi-Fi...");
    esp_wifi_connect();
}

// Initialize Camera
static esp_err_t init_camera(void)
{
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    ESP_LOGI(TAG, "Camera initialized successfully");
    return ESP_OK;
}

// Check if the sensor is detected
void detect_sensor(void)
{
    sensor_t *sensor = esp_camera_sensor_get();
    if (!sensor) {
        ESP_LOGE(TAG, "Sensor not detected!");
        return;
    }

    ESP_LOGI(TAG, "Sensor detected!");
    return;
}

// Send image via HTTP Post
static esp_err_t send_image_to_server(camera_fb_t *fb)
{
    esp_http_client_config_t config = {
        .url = SERVER_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 20000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "image/jpeg");
    esp_http_client_set_post_field(client, (const char *)fb->buf, fb->len);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP Post status: %d", esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "HTTP Post failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}

// Main task to capture and send frames
void capture_and_send_task(void *pvParameters)
{
    static int error_count;

    while (1) {
        // Capture an image
        ESP_LOGI(TAG, "Capturing image...");
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb)
        {
            ESP_LOGE(TAG, "Failed to capture frame");
            // Reset camera on repeated failures
            error_count = 0;
            if (++error_count > 3) {
                esp_restart();
            }
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }
        error_count = 0; // Reset error counter on success

        // Add JPEG header check
        if (fb->len < 100 || fb->buf[0] != 0xFF || fb->buf[1] != 0xD8) {
            ESP_LOGE(TAG, "Invalid JPEG image");
            esp_camera_fb_return(fb);
            continue;
        }

        esp_err_t ret = send_image_to_server(fb);
        // Release the frame buffer
        esp_camera_fb_return(fb);

        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Delaying due to send failure");
            // Delay before taking the next picture
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        } else {
            // Delay before taking the next picture
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}

void app_main() {

    // Initialize NVS (required for Wi-Fi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Erase NVS partition and reinitialize
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize WIFI
    init_wifi();

    // Initialize the camera
    ESP_ERROR_CHECK(init_camera());

    // Check if the sensor is detected
    detect_sensor();

    // Capture a frame
    xTaskCreate(capture_and_send_task, "capture_and_send_task", 4096, NULL, 5, NULL);

}
