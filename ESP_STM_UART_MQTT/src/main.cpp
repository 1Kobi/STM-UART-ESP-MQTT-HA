#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>

const char* ssid = "xxx";
const char* password = "xxx";
const char* mqtt_server = "xxx";
const char* mqtt_user = "xxx";
const char* mqtt_pass = "xxx";

const char* topic_telemetry = "moj_dom/salon/telemetria/stm32";

// Adres pliku z nowym programem dla STM32
String file_url = "http://192.168.1.21:8000/Czujniki_WIFI.bin";
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

// ==========================================
// FUNKCJA OTA: Wgrywa kod do STM32 przy starcie
// ==========================================
void performOTA() {
  Serial.println(">>> Czekam na sygnal START od STM32...");
  
  // Czekamy, aż STM32 wyśle dowolny znak (np. 'S')
  while (true) {
    if (stmSerial.available() > 0) {
      char c = stmSerial.read();
      if (c == 'S') break; // Dostaliśmy sygnał, ruszamy!
    }
    delay(10);
  }

  Serial.println(">>> Sygnal odebrany! Pobieram plik z serwera...");
  // ... reszta kodu http.GET() bez zmian ...

  Serial.println(">>> Sprawdzam aktualizacje na serwerze Pythonowym...");
  WiFiClient WIFIclient;
  HTTPClient http;
  
  http.begin(WIFIclient, file_url);
  int httpCode = http.GET();

  // Jeśli serwer odpowiedział poprawnie i plik istnieje
  if (httpCode == HTTP_CODE_OK) {
    int len = http.getSize();
    Serial.printf(">>> Znaleziono nowy kod! Rozmiar: %d bajtow.\n", len);

    WiFiClient * stream = http.getStreamPtr();
    uint8_t buffer[64];
    int bytesSent = 0;

    // Pobieramy paczki z WiFi i wypychamy kabelkiem do STM32
    while (http.connected() && (len > 0 || len == -1)) {
      size_t size = stream->available();
      if (size) {
        int c = stream->readBytes(buffer, ((size > sizeof(buffer)) ? sizeof(buffer) : size));
        for(int i = 0; i < c; i++) {
          stmSerial.write(buffer[i]); // Wysyłamy pojedynczy bajt
          delay(2); // Czekamy 2 ms, aby STM32 zdążyło zapisać go w krzemie
        }
        bytesSent += c;
        if (len > 0) len -= c;
      }
      delay(1);
    }
    Serial.printf("\n>>> SUKCES: Wyslano caly plik do STM32 (%d bajtow)!\n", bytesSent);
  } else {
    // Serwer wyłączony lub brak pliku
    Serial.printf(">>> Brak aktualizacji (Kod HTTP: %d). Zwykly start.\n", httpCode);
  }
  http.end();
}

void setup() {
  // Port do komunikacji z komputerem (logi)
  Serial.begin(115200);
  
  // Port do komunikacji z STM32 (9600 to bezpieczna prędkość dla zapisu Flash!)
  stmSerial.begin(9600);
  
  setupWiFi();

  // 1. NAJPIERW SPRAWDZAMY AKTUALIZACJĘ DLA STM32
  performOTA();

  // 2. POTEM KONFIGURUJEMY MQTT
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

  // 2. Odbieranie telemetrii z STM32 (Zmieniono na stmSerial!)
  if (stmSerial.available() > 0) {
    String incomingData = stmSerial.readStringUntil('\n');
    incomingData.trim();
    
    Serial.println(">>> RAW DATA Z STM32: [" + incomingData + "]");

    if (client.connected() && incomingData.length() > 0) {
      client.publish(topic_telemetry, incomingData.c_str());
    }
  }
}