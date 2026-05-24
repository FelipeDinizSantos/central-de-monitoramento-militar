#include "WiFi.h"
#include "PubSubClient.h"
#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"
#include "time.h"
#include <HTTPClient.h>

// API LARAVEL URL
const char* API_URL = "https://laravel-api-production-ebab.up.railway.app/api/registros-acesso";

// WIFI 
const char* WIFI_SSID = "ARYA-DEUSA";
const char* WIFI_PASSWORD = "felipeBom123";

// MQTT 
const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC = "felipe-diniz/projeto/exercito/sensor-distancia"; // Igual do publish
String clientId = "esp32cam_" + String((uint32_t)ESP.getEfuseMac(), HEX); // Tanto faz o cliente que usar

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// NTP - Para camera 
const char* NTP_SERVER = "pool.ntp.org"; // usado para sincronizar o horario 
unsigned long lastReconnectAttempt = 0;
unsigned long lastPhotoTime = 0;

const unsigned long PHOTO_COOLDOWN = 10000;

// PINS da camera 
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

// Conexão WIFI 
void connectWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Conectando WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP());
}

// Configura data e hora
void setupTime()
{
  configTzTime(
    "<-03>3",
    "pool.ntp.org"
  );

  struct tm timeinfo;

  Serial.print("Sincronizando horario");

  while (!getLocalTime(&timeinfo))
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("Horario sincronizado");

  Serial.println(&timeinfo, "%d/%m/%Y %H:%M:%S");
}

String getTimestamp()
{
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo))
  {
    return "sem_horario";
  }

  char buffer[40];

  strftime(
    buffer,
    sizeof(buffer),
    "%Y%m%d_%H%M%S",
    &timeinfo
  );

  return String(buffer);
}

// Inicia a camera  
bool initCamera()
{
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
  config.grab_mode = CAMERA_GRAB_LATEST;

  if (psramFound())
  {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
  }
  else
  {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK)
  {
    Serial.printf("Erro camera: 0x%x\n", err);
    return false;
  }

  return true;
}

// Inicia o cartão SD 
bool initSD()
{
  if (!SD_MMC.begin())
  {
    Serial.println("Erro ao iniciar SD");
    return false;
  }

  uint8_t cardType = SD_MMC.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("Nenhum SD encontrado");
    return false;
  }

  Serial.println("SD iniciado");
  return true;
}

// Envia para API Laravel local 
void sendPhotoToAPI(camera_fb_t* fb)
{
  HTTPClient http;

  http.begin(API_URL);

  http.addHeader("Content-Type", "image/jpeg");

  http.addHeader("device-id", clientId);

  http.addHeader("mensagem", "Movimento detectado");

  Serial.print("Tamanho imagem: ");
  Serial.println(fb->len);

  Serial.print("Conectado WiFi: ");
  Serial.println(WiFi.status() == WL_CONNECTED);

  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());

  Serial.print("API URL: ");
  Serial.println(API_URL);
  int responseCode = http.POST(fb->buf, fb->len);

  Serial.print("HTTP Response: ");
  Serial.println(responseCode);
  
  if (responseCode > 0)
  {
    String response = http.getString();

    Serial.println(response);
  }
  else
  {
    Serial.print("Erro HTTP: ");
    Serial.println(http.errorToString(responseCode));
  }

  http.end();

  Serial.print("ESP IP: ");
  Serial.println(WiFi.localIP());
}

// FOTO 
void capturePhoto()
{
  unsigned long now = millis();

  if (now - lastPhotoTime < PHOTO_COOLDOWN)
  {
    Serial.println("Cooldown ativo");
    return;
  }

  lastPhotoTime = now;

  Serial.println("Limpando buffer antigo...");

  camera_fb_t* oldFb = esp_camera_fb_get();

  if (oldFb)
  {
    esp_camera_fb_return(oldFb);
  }

  delay(100);

  Serial.println("Capturando foto atual...");

  camera_fb_t* fb = esp_camera_fb_get();

  if (!fb)
  {
    Serial.println("Falha captura");
    return;
  }

  Serial.println("Enviando para API...");

  sendPhotoToAPI(fb);

  esp_camera_fb_return(fb);

  Serial.println("Upload finalizado");
}

void mqttCallback(
  char* topic,
  byte* payload,
  unsigned int length
)
{
  Serial.print("Mensagem recebida em: ");
  Serial.println(topic);

  String message;

  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  Serial.print("Payload: ");
  Serial.println(message);

  capturePhoto(); // qualquer mensagem recebida tira foto
}

// MQTT 
bool connectMQTT()
{
  if (mqttClient.connected())
  {
    return true;
  }

  Serial.print("Conectando MQTT... ");

  bool connected = mqttClient.connect(clientId.c_str());

  if (connected)
  {
    Serial.println("conectado");

    mqttClient.subscribe(MQTT_TOPIC);

    Serial.print("Inscrito em: ");
    Serial.println(MQTT_TOPIC);
  }
  else
  {
    Serial.print("falhou rc=");
    Serial.println(mqttClient.state());
  }

  return connected;
}

// SETUP 
void setup()
{
  Serial.begin(19200);

  delay(1000);

  connectWiFi();

  delay(2000);

  setupTime();

  if (!initCamera())
  {
    Serial.println("Falha camera");
    return;
  }

  if (!initSD())
  {
    Serial.println("Falha SD");
    return;
  }

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

  mqttClient.setCallback(mqttCallback);

  connectMQTT();

  Serial.println("Sistema pronto");
}

// LOOP 
void loop()
{
  if (!mqttClient.connected())
  {
    unsigned long now = millis();

    if (now - lastReconnectAttempt > 5000)
    {
      lastReconnectAttempt = now;

      connectMQTT();
    }
  }
  else
  {
    mqttClient.loop();
  }

  delay(10);
}