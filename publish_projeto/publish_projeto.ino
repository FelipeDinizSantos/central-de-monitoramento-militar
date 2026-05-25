#include <SPI.h>
#include <WiFiEsp.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

// -------------------- SENSOR --------------------
const int TRIGGER_PIN = 4;
const int ECHO_PIN = 5;
const int BUZZER_PIN = 10;

// -------------------- WIFI --------------------
char SSID[] = "FelipeDinizTeste";
char PASSWORD[] = "12345678";

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
const int DIST_VERY_NEAR = 70;
const unsigned long SENSOR_INTERVAL = 500;

const int REQUIRED_CONFIRMATIONS = 2;
const unsigned long ULTRASONIC_TIMEOUT = 15000;

// -------------------- ESTADO --------------------
double distanceCm = 0;

unsigned long lastSensorRead = 0;

bool veryNearSent = false;

unsigned long lastReconnectWiFi = 0;
unsigned long lastReconnectMQTT = 0;

int confirmCount = 0;

// -------------------- ULTRASSÔNICO --------------------
long readUltrasonic()
{
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIGGER_PIN, LOW);

  // Timeout reduzido
  return pulseIn(ECHO_PIN, HIGH, ULTRASONIC_TIMEOUT);
}

double getDistanceCm()
{
  long duration = readUltrasonic();

  if (duration == 0)
    return -1;

  double cm = duration * 0.01723;

  if (cm < 2 || cm > 400)
    return -1;

  return cm;
}

// -------------------- WIFI --------------------
void connectWiFi()
{
  Serial.println("Conectando WiFi...");

  unsigned long startAttempt = millis();

  while (
    WiFi.status() != WL_CONNECTED &&
    millis() - startAttempt < 10000
  )
  {
    WiFi.begin(SSID, PASSWORD);

    unsigned long waitStart = millis();

    while (
      WiFi.status() != WL_CONNECTED &&
      millis() - waitStart < 3000
    )
    {
      mqttClient.loop();
      yield();
      delay(100);
    }
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi conectado");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Falha ao conectar WiFi");
  }
}

// -------------------- MQTT --------------------
void connectMQTT()
{
  if (WiFi.status() != WL_CONNECTED)
    return;

  Serial.println("Conectando MQTT...");

  unsigned long startAttempt = millis();

  while (
    !mqttClient.connected() &&
    millis() - startAttempt < 10000
  )
  {
    if (mqttClient.connect("arduino-uno-sensor"))
    {
      Serial.println("MQTT conectado");
      return;
    }

    Serial.print("Erro MQTT: ");
    Serial.println(mqttClient.state());

    unsigned long waitStart = millis();

    while (millis() - waitStart < 2000)
    {
      mqttClient.loop();
      yield();
      delay(50);
    }
  }

  Serial.println("Falha ao conectar MQTT");
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
  unsigned long delayStart = millis();

  while (millis() - delayStart < 1000)
  {
    mqttClient.loop();
    yield();
    delay(10);
  }

  tone(BUZZER_PIN, 2200);

  unsigned long start = millis();

  while (millis() - start < 150)
  {
    mqttClient.loop();
    yield();
    delay(5);
  }

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

    while (true)
    {
      yield();
    }
  }

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

  // Mais tolerância
  mqttClient.setKeepAlive(60);

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

  // ---------------- MQTT LOOP ----------------
  mqttClient.loop();
  yield();

  // ---------------- WiFi reconnect ----------------
  if (WiFi.status() != WL_CONNECTED)
  {
    if (now - lastReconnectWiFi > 5000)
    {
      Serial.println("WiFi desconectado");

      connectWiFi();

      lastReconnectWiFi = now;
    }

    return;
  }

  // ---------------- MQTT reconnect ----------------
  if (!mqttClient.connected())
  {
    if (now - lastReconnectMQTT > 5000)
    {
      Serial.println("MQTT desconectado");

      connectMQTT();

      lastReconnectMQTT = now;
    }
  }

  // ---------------- SENSOR ----------------
  if (now - lastSensorRead >= SENSOR_INTERVAL)
  {
    lastSensorRead = now;

    distanceCm = getDistanceCm();

    // -------- ALERTA --------
    if (
      distanceCm > 0 &&
      distanceCm <= DIST_VERY_NEAR
    )
    {
      confirmCount++;

      Serial.print("Confirmacoes: ");
      Serial.println(confirmCount);

      if (
        confirmCount >= REQUIRED_CONFIRMATIONS &&
        !veryNearSent
      )
      {
        publishVeryNear(distanceCm);

        beepOnce();

        veryNearSent = true;
      }
    }
    else
    {
      confirmCount = 0;
      veryNearSent = false;
    }
  }
}