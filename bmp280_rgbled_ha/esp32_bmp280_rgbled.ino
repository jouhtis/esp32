#include "valot.h"
#include "sensorit.h"
#include "wifi_mqtt.h"

void setup() {
  Serial.println("Käynnistetään laite...");
  Serial.begin(115200);
  setupLedi();
  setupBMP();
  setupWiFi();
  setupMQTT();
}

void loop() {
  // Pidetään MQTT yhteys elossa
  if (!client.connected()) setupMQTT();
  client.loop();

  float t = lueLampotila();
  paivitaLedi(t);

  static unsigned long lastSerialPrint = 0;
  if (millis() - lastSerialPrint > 5000) {
    lastSerialPrint = millis();
    Serial.print("Lämpötila: ");
    Serial.print(t);
    Serial.println(" °C");
  }
  // Lähetys HA:han esim. 30s välein
  static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 30000) {
    lastMsg = millis();
    client.publish("home/sensor/esp32_temp/state", String(t).c_str());
  }
}