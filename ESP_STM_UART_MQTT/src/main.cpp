#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

const char* ssid = "xxx";
const char* password = "xxx";
const char* mqtt_server = "xxx";
const char* mqtt_user = "xxx";
const char* mqtt_pass = "xxx";

const char* topic_telemetry = "moj_dom/salon/telemetria/stm32";

SoftwareSerial stmSerial(D5, D6);

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastReconnectAttempt = 0;

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("\n>>> Laczenie z WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n>>> WiFi podlaczone!");
}

void reconnect() {
  if (millis() - lastReconnectAttempt < 5000) {
    return;
  }
  lastReconnectAttempt = millis();

  if (!client.connected() && WiFi.status() == WL_CONNECTED) {
    String clientId = "ESP8266-Gateway-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println(">>> Polaczono z MQTT!");
    }
  }
}

void setup() {
  Serial.begin(9600);
  stmSerial.begin(9600);

  setupWiFi();

  client.setServer(mqtt_server, 1883);
  Serial.println(">>> ESP gotowe do obslugi MQTT i telemetrii!");
}

void loop() {
  // 1. Utrzymanie połączenia MQTT
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop(); 
  }

  // 2. Odbieranie telemetrii z STM32
  if (stmSerial.available() > 0) {
    String incomingData = stmSerial.readStringUntil('\n');
    incomingData.trim();
    
    if (incomingData.length() > 0) {
      Serial.println("STM32: [" + incomingData + "]");
      
      if (client.connected()) {
        client.publish(topic_telemetry, incomingData.c_str());
      }
    }
  }
}
