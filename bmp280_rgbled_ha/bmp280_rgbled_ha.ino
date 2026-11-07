#include "valot.h"
#include "sensorit.h"
#include "wifi_mqtt.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Käynnistetään laite...");
  setupLedi();
  testaaLedinSkaala();
  setupBMP();
  paivitaLedi(lueLampotila());
  setupWiFi();
  setupMQTT();
}

void loop() {
  float t = lueLampotila();
  paivitaLedi(t);

  static unsigned long lastSerialPrint = 0;
  if (millis() - lastSerialPrint > 5000) {
    lastSerialPrint = millis();
    Serial.print("Lämpötila: ");
    Serial.print(t);
    Serial.println(" °C");
  }
  yllapidaMQTT();

  // Lähetys HA:han esim. 30s välein
  static unsigned long lastMsg = 0;
  if (client.connected() && millis() - lastMsg > 30000) {
    lastMsg = millis();
    char timestamp[32];
    if (utcTimestamp(timestamp, sizeof(timestamp))) {
      String payload = "{\"sensor_id\":\"bmp280_sisatila\",\"location\":\"sisatila\",\"measured_at\":\"";
      payload += timestamp;
      payload += "\",\"schema\":2,\"values\":{\"temperature\":";
      payload += String(t, 2);
      payload += "},\"units\":{\"temperature\":\"°C\"}}";
      client.publish(slow_state_topic, payload.c_str());
    } else {
      Serial.println("NTP-aikaa ei ole vielä saatavilla; MQTT-julkaisu ohitetaan.");
    }
  }
}
