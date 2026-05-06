#include <WiFi.h>
#include <PubSubClient.h>
#include "env.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER;
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASSWORD;

WiFiClient espClient;
PubSubClient client(espClient);

const char* state_topic = "home/sensor/esp32_temp/state";
const char* config_topic = "homeassistant/sensor/esp32_temp/config";

// TÄMÄ FUNKTIO PUUTTUI TAI OLI VÄÄRÄSSÄ PAIKASSA
void lahetaDiscovery() {
String payload = "{"
    "\"name\": \"TV taso Lämpötila\","
    "\"stat_t\": \"home/sensor/esp32_temp/state\","
    "\"unit_of_meas\": \"°C\","
    "\"dev_cla\": \"temperature\","
    "\"val_tpl\": \"{{ value }}\","
    "\"uniq_id\": \"esp32s3_bmp280_01\","
    "\"dev\": {\"ids\": [\"esp32s3_01\"], \"name\": \"ESP32 S3 Mittari\"}"
  "}";

if (client.publish(config_topic, payload.c_str(), true)) {
    Serial.println("Discovery ONNISTUI!");
  } else {
    Serial.println("Discovery EPÄONNISTUI - puskuri liian pieni?");
  }
}

void setupWiFi() {
  Serial.print("Yhdistetään WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi yhdistetty!");
}

void setupMQTT() {
  client.setServer(mqtt_server, 1883);
  client.setBufferSize(512); // Nosta puskurin kokoa Discovery-viestiä varten!
  client.setServer(mqtt_server, 1883);
  
  while (!client.connected()) {
    Serial.print("Yhdistetään MQTT...");
    String clientId = "ESP32S3-Temp-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("yhdistetty!");
      lahetaDiscovery(); // Nyt kääntäjä löytää tämän, koska se on määritelty ylempänä
    } else {
      Serial.print("virhe, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}
