#include "esp_camera.h"
#include <WiFi.h>

//
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled
//

#include "camera_pins.h"

const char* ssid = "######";
const char* password = "######";

uint32_t startlight = 0;

TaskHandle_t flashLightTask;

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

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
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

  WiFi.begin(ssid, password);
  
  reconnect();

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  pinMode(4, OUTPUT);
  digitalWrite(4,0);

  xTaskCreatePinnedToCore( doFlashLight, "flashTask", 10000, NULL, 1, &flashLightTask, 0);

}

void reconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    int reconnectCount = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      if (reconnectCount > 5) {
        Serial.println("restarting wifi");
        WiFi.mode(WIFI_OFF);
        delay(1000);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        reconnectCount = 0;
      }
      delay(1000);
      Serial.print(".");
      reconnectCount++;
    }
    Serial.println("");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}
void loop() {
  // put your main code here, to run repeatedly:
  vTaskDelay(10000);
  reconnect();
}

void doFlashLight( void * pvParameters ) {
  for (;;) {
//    Serial.print("flashlight start: ");
//    Serial.println(startlight);
    if ((startlight > 0) && (startlight < millis())) {
      digitalWrite(4,0);
      startlight = 0;
    }
    vTaskDelay(1000);
  }
}
