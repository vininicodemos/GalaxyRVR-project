#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h> 

// REDE COM ACESSO À INTERNET
const char* ssid = "Vinicius"; 
const char* password = "#Nicodemos1961";

// SERVIDOR NA NUVEM
String serverUrl = "https://camera-esp.onrender.com/upload";

// PINOS
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

// Criamos o cliente SEGURO global
WiFiClientSecure client;
HTTPClient http;

void setup() {
  Serial.begin(115200);
  
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
  
  config.xclk_freq_hz = 10000000; 
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Imagem em tamanho QVGA (320x240) e qualidade com maior compressão (20)
  config.frame_size = FRAMESIZE_HVGA; 
  config.jpeg_quality = 14; 
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Erro na lente!");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado a Internet!");

  // Ignora a verificação do certificado para não perder tempo
  client.setInsecure(); 
  
  // Mantém a porta aberta na nuvem
  http.setReuse(true); 
}

void loop() {
  if(WiFi.status() == WL_CONNECTED){
    camera_fb_t * fb = esp_camera_fb_get();
    if(!fb) {
      Serial.println("Falha na captura");
      return;
    }

    // Inicia a conexão passando o cliente seguro
    http.begin(client, serverUrl); 
    http.addHeader("Content-Type", "image/jpeg");
    
    int httpResponseCode = http.POST(fb->buf, fb->len);
    
    if(httpResponseCode > 0){
      Serial.print("Enviado super rapido! Codigo HTTP: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Falha na internet: ");
      Serial.println(httpResponseCode);
    }
    
    // Fecha a porta corretamente após a entrega para o ESP32 respirar
    http.end(); 
    
    esp_camera_fb_return(fb);
  }
  
  // Como a foto agora é leve, podemos enviar bem mais rápido sem estourar o buffer
  delay(50); 
}
