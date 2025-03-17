#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"  // Asegura incluir esta biblioteca

// Configuración de la cámara para ESP32-CAM (AI-THINKER)
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

const char* ssid = "ChatESP32";  // Red del ESP-WROOM-32
const char* password = "";       // Sin contraseña

IPAddress local_IP(1, 1, 1, 2);
IPAddress gateway(1, 1, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Función para manejar el streaming de la cámara
esp_err_t stream_handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len;
    uint8_t *_jpg_buf;
    char part_buf[64];

    res = httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");
    if (res != ESP_OK) return res;

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Error al obtener la imagen");
            res = ESP_FAIL;
        } else {
            _jpg_buf = fb->buf;
            _jpg_buf_len = fb->len;
            sprintf(part_buf, "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n", _jpg_buf_len);
            res = httpd_resp_send_chunk(req, part_buf, strlen(part_buf));
            if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
            if (res == ESP_OK) res = httpd_resp_send_chunk(req, "\r\n", 2);
            esp_camera_fb_return(fb);
        }
        if (res != ESP_OK) break;
    }
    return res;
}

// Función para iniciar el servidor de la cámara
void startCameraServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 81;
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t stream_uri = {
            .uri = "/stream",
            .method = HTTP_GET,
            .handler = stream_handler,  // Asigna la función correctamente
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &stream_uri);
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.config(local_IP, gateway, subnet);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConectado a la red " + String(ssid));

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.println("Error al iniciar la cámara");
        return;
    }
    
    startCameraServer();  // Inicia el servidor de la cámara
}

void loop() {}
