# Smart Home IoT Sensor Node (STM32 + ESP8266)

Projekt wykorzystuje architekturę dwuprocesorową: STM32 odpowiada za rygorystyczne zadania czasu rzeczywistego (RTOS) i akwizycję danych, a ESP8266 pełni rolę dedykowanej bramy sieciowej (Gateway) WiFi/MQTT.

## 🏗️ Architektura Systemu

System został podzielony na dwie niezależne jednostki, komunikujące się ze sobą poprzez magistralę UART:

1. **STM32 (Main Controller):** Odpowiada za odczyt środowiskowy z czujnika BMP280. Działa pod kontrolą systemu **FreeRTOS**, zarządzając zadaniami pomiaru i komunikacji.
2. **ESP8266 (WiFi Gateway):** Odbiera sformatowane ramki JSON od STM32 i asynchronicznie publikuje je na serwerze MQTT, udostępniając dane systemom Smart Home.

## 🚀 Kluczowe Technologie i Funkcjonalności

* **FreeRTOS (Zarządzanie Wątkami i Kolejkami):** Zastosowano wielowątkowość. `TempTask` odczytuje dane i umieszcza je w bezpiecznej kolejce IPC (`osMessageQueue`), a `UartTask` asynchronicznie wyciąga je i formatuje.
* **Direct Memory Access (DMA):** Transmisja UART z STM32 do ESP8266 realizowana jest przez sprzętowy kontroler DMA (`HAL_UART_Transmit_DMA`), co całkowicie odciąża główny procesor podczas wysyłania payloadu.
* **Independent Watchdog (IWDG):** System jest automatycznie resetowany w przypadku awarii szyny I2C lub zawieszenia się wątku pomiarowego.
* **Formatowanie JSON & MQTT:** Dane są pakowane do standardu JSON (np. `{"temperature": 23.50}`) i wysyłane z zachowaniem mechanizmów autoconnect na broker MQTT.
