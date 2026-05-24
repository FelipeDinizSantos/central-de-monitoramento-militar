#include <SPI.h>
#include <RF24.h>
#include <WiFiEsp.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

// -------------------- SENSOR --------------------
const int TRIGGER_PIN = 4;
const int ECHO_PIN = 5;
const int BUZZER_PIN = 10;

// -------------------- NRF24 --------------------
const int CE_PIN = 7;
const int CSN_PIN = 8;

RF24 radio(CE_PIN, CSN_PIN);

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
const int DIST_MAX = 300;
const int DIST_MED = 200;
const int DIST_NEAR = 100;
const int DIST_VERY_NEAR = 20;

// Intervalo aumentado para maior estabilidade
const unsigned long SENSOR_INTERVAL = 500;

// -------------------- ESTADO --------------------
double distanceCm = 0;

unsigned long lastSensorRead = 0;

bool veryNearSent = false;

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
  {
    return -1;
  }

  return duration * 0.01723;
}

// -------------------- WIFI --------------------
void connectWiFi()
{
  Serial.println("Conectando WiFi...");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Tentando conectar...");

    WiFi.begin(SSID, PASSWORD);

    delay(10000);
  }

  Serial.println("WiFi conectado");

  Serial.print("IP: ");

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

      delay(3000);
    }
  }
}

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

  // Pequeno delay para estabilidade do ESP8266
  delay(100);

  if (ok)
  {
    Serial.println("Evento MQTT enviado");
  }
  else
  {
    Serial.println("Falha ao publicar MQTT");
  }
}

// -------------------- BUZZER --------------------
void beepOnce()
{
  tone(BUZZER_PIN, 2200);

  delay(150);

  noTone(BUZZER_PIN);
}

// -------------------- SERIAL --------------------
void logSerial(double cm)
{
  if (cm < 0)
  {
    Serial.println("Erro");
  }
  else
  {
    Serial.print(cm);

    Serial.println(" cm");
  }
}

// -------------------- SETUP --------------------
void setup()
{
  Serial.begin(9600);

  // IMPORTANTE:
  // O ESP8266 deve estar configurado em 9600 baud:
  // AT+UART_DEF=9600,8,1,0,0
  espSerial.begin(9600);

  WiFi.init(&espSerial);

  // Verifica comunicação com ESP8266
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("ESP8266 nao encontrado");

    while (true);
  }

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);

  // Ajustes de estabilidade MQTT
  mqttClient.setKeepAlive(30);

  mqttClient.setSocketTimeout(30);

  pinMode(TRIGGER_PIN, OUTPUT);

  pinMode(ECHO_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  // NRF24 inicializado mas sem uso ativo
  radio.begin();

  radio.setPALevel(RF24_PA_LOW);

  connectWiFi();

  connectMQTT();

  delay(1000);
}

// -------------------- LOOP --------------------
void loop()
{
  // Reconexão WiFi
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi desconectado");

    connectWiFi();
  }

  // Reconexão MQTT
  if (!mqttClient.connected())
  {
    Serial.println("MQTT desconectado");

    connectMQTT();
  }

  mqttClient.loop();

  unsigned long now = millis();

  if (now - lastSensorRead >= SENSOR_INTERVAL)
  {
    lastSensorRead = now;

    distanceCm = getDistanceCm();

    if (
      distanceCm > 0 &&
      distanceCm <= DIST_VERY_NEAR
    )
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