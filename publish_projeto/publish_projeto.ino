#include <SPI.h>
#include <WiFiEsp.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

// -------------------- SENSOR --------------------
const int TRIGGER_PIN = 4;
const int ECHO_PIN = 5;
const int BUZZER_PIN = 10;

// -------------------- WIFI --------------------
char SSID[] = "ARYA-DEUSA";
char PASSWORD[] = "felipeBom123";

// RX, TX
SoftwareSerial espSerial(2, 3);

WiFiEspClient wifiClient;
PubSubClient mqttClient(wifiClient);

// -------------------- MQTT --------------------
const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

const char* MQTT_TOPIC =
  "felipe-diniz/projeto/exercito/sensor-distancia";

// -------------------- CONFIGURAÇÕES --------------------
const int DIST_VERY_NEAR = 20;
const unsigned long SENSOR_INTERVAL = 500;

// -------------------- ESTADO --------------------
double distanceCm = 0;

unsigned long lastSensorRead = 0;
bool veryNearSent = false;

unsigned long lastReconnectWiFi = 0;
unsigned long lastReconnectMQTT = 0;

// -------------------- ULTRASSÔNICO --------------------
long readUltrasonic()
{
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIGGER_PIN, LOW);

  return pulseIn(ECHO_PIN, HIGH, 30000);
}

double getDistanceCm()
{
  long duration = readUltrasonic();

  if (duration == 0)
    return -1;

  return duration * 0.01723;
}

// -------------------- WIFI --------------------
void connectWiFi()
{
  Serial.println("Conectando WiFi...");

  while (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(SSID, PASSWORD);
    delay(5000);
    yield();
  }

  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP());
}

// -------------------- MQTT --------------------
void connectMQTT()
{
  while (!mqttClient.connected())
  {
    Serial.println("Conectando MQTT...");

    if (mqttClient.connect("arduino-uno-sensor"))
    {
      Serial.println("MQTT conectado");
    }
    else
    {
      Serial.print("Erro MQTT: ");
      Serial.println(mqttClient.state());
      delay(2000);
      yield();
    }
  }
}

// -------------------- PUBLISH --------------------
void publishVeryNear(double cm)
{
  char payload[64];

  snprintf(
    payload,
    sizeof(payload),
    "ALERTA: objeto muito proximo - %.2f cm",
    cm
  );

  bool ok = mqttClient.publish(MQTT_TOPIC, payload);

  if (ok)
    Serial.println("Evento MQTT enviado");
  else
    Serial.println("Falha ao publicar MQTT");
}

// -------------------- BUZZER --------------------
void beepOnce()
{
  tone(BUZZER_PIN, 2200);
  delay(150);
  noTone(BUZZER_PIN);
}

// -------------------- SETUP --------------------
void setup()
{
  Serial.begin(9600);

  espSerial.begin(9600);
  WiFi.init(&espSerial);

  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("ESP8266 nao encontrado");
    while (true);
  }

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setKeepAlive(30);
  mqttClient.setSocketTimeout(30);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  connectWiFi();
  connectMQTT();
}

// -------------------- LOOP --------------------
void loop()
{
  unsigned long now = millis();

  // ---------------- WiFi reconnect control ----------------
  if (WiFi.status() != WL_CONNECTED)
  {
    if (now - lastReconnectWiFi > 5000)
    {
      Serial.println("WiFi desconectado");
      connectWiFi();
      lastReconnectWiFi = now;
    }
  }

  // ---------------- MQTT reconnect control ----------------
  if (!mqttClient.connected())
  {
    if (now - lastReconnectMQTT > 5000)
    {
      Serial.println("MQTT desconectado");
      connectMQTT();
      lastReconnectMQTT = now;
    }
  }

  mqttClient.loop();
  yield();

  // ---------------- SENSOR ----------------
  if (now - lastSensorRead >= SENSOR_INTERVAL)
  {
    lastSensorRead = now;

    distanceCm = getDistanceCm();

    if (distanceCm > 0 && distanceCm <= DIST_VERY_NEAR)
    {
      if (!veryNearSent)
      {
        beepOnce();
        publishVeryNear(distanceCm);
        veryNearSent = true;
      }
    }
    else
    {
      veryNearSent = false;
    }
  }
}