#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

const char* ssid = "xxx";
const char* password = "xxx";
const char* mqtt_server = "xxx";
const char* mqtt_user = "xxx";
const char* mqtt_pass = "xxx";

const char* topic_telemetry = "moj_dom/salon/telemetria/stm32";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastReconnectAttempt = 0;

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // Tutaj delay jest ok, bo bez WiFi i tak nic nie zrobimy
  }
}

void reconnect() {
  if (millis() - lastReconnectAttempt < 5000) {
    return;
  }
  lastReconnectAttempt = millis();

  if (!client.connected() && WiFi.status() == WL_CONNECTED) {
    String clientId = "ESP8266-Gateway-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      // Polaczono!
    }
  }
}

void setup() {
  // UWAGA: Inicjalizujemy UART z prędkością 115200 (musi być Taka sama jak w STM32!)
  Serial.begin(115200);
  
  setupWiFi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  // 1. Utrzymanie połączenia (bez zmian)
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop(); 
  }

  // 2. Odbieranie danych z STM32
  if (Serial.available() > 0) {
    String incomingData = Serial.readStringUntil('\n');
    incomingData.trim();
    
    Serial.println(">>> RAW DATA Z STM32: [" + incomingData + "]");

    if (client.connected() && incomingData.length() > 0) {
      Serial.println(">>> Wysylam powyzsze dane do MQTT...");
      client.publish(topic_telemetry, incomingData.c_str());
    }
  }
}