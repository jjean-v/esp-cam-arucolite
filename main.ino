#define ARUCO_DB ARUCO_DB_4X4_1000
#define ARUCO_DB_SIZE 100
#include <ArucoLite.h>
#include "esp_camera.h"

// Brochage ESP32-CAM AI-Thinker
#define PWDN_GPIO_NUM     32
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

#define W 320
#define H 240

typedef ArucoLite<W, H, 16, false> MyAruco_t;
static MyAruco_t Aruco;

void setup() {
  Serial.begin(115200);

  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE;
  config.frame_size   = FRAMESIZE_QVGA;      // 320x240
  config.fb_count     = 1;
  config.fb_location  = CAMERA_FB_IN_PSRAM;
  config.grab_mode    = CAMERA_GRAB_LATEST;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Erreur init caméra : 0x%x\n", err);
    while (true) delay(1000);
  }
  Serial.println("Caméra OK");
}

void loop() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) { Serial.println("Capture ratée"); delay(100); return; }

  if (fb->format != PIXFORMAT_GRAYSCALE || fb->width != W || fb->height != H) {
    Serial.printf("Mauvais format: %dx%d fmt=%d\n", fb->width, fb->height, fb->format);
    esp_camera_fb_return(fb);
    delay(500);
    return;
  }

  // Le buffer caméra a exactement la forme d'un Frame : on le réutilise tel quel
  MyAruco_t::Frame *f = reinterpret_cast<MyAruco_t::Frame *>(fb->buf);
  Aruco.process(*f);

  esp_camera_fb_return(fb);   // rendre le buffer seulement APRÈS process()

  Serial.printf("found %d arucos\n", Aruco.arucos_found);
  for (int i = 0; i < Aruco.arucos_found; i++) {
    aruco_t &a = Aruco.result[i];
    Serial.printf("id %d : ", a.aruco_idx);
    for (int p = 0; p < 4; p++)
      Serial.printf("(%g, %g) ", a.pt[p].x, a.pt[p].y);
    Serial.println();
  }
}