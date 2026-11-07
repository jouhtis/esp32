#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include "env.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER;
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASSWORD;

WiFiClient espClient;
PubSubClient client(espClient);

const char* config_topic = "homeassistant/sensor/esp32_temp/config";
const char* slow_state_topic = "sensors/bmp280_sisatila/state";

bool utcTimestamp(char* output, size_t outputSize) {
  time_t now = time(nullptr);
  if (now < 1700000000) return false;
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  return strftime(output, outputSize, "%Y-%m-%dT%H:%M:%SZ", &timeinfo) > 0;
}

void lahetaDiscovery() {
String payload = "{"
    "\"name\": \"TV taso Lämpötila\","
    "\"stat_t\": \"sensors/bmp280_sisatila/state\","
    "\"unit_of_meas\": \"°C\","
    "\"dev_cla\": \"temperature\","
    "\"val_tpl\": \"{{ value_json.values.temperature }}\","
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
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

void setupMQTT() {
  client.setServer(mqtt_server, 1883);
  client.setBufferSize(512);
  // Rajoita yksittäisen synkronisen yhteysyrityksen odotuksia.
  espClient.setConnectionTimeout(250); // TCP-yhteys, millisekuntia
  client.setSocketTimeout(1);          // MQTT-vastaus, sekuntia
}

void yllapidaMQTT() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (client.connected()) {
    client.loop();
    return;
  }

  static uint8_t failedAttempts = 0;
  static unsigned long lastAttempt = 0;
  if (failedAttempts >= 10) return;
  if (failedAttempts > 0 && millis() - lastAttempt < 60000UL) return;

  Serial.print("Yhdistetään MQTT...");
  String clientId = "ESP32S3-Temp-";
  clientId += String(random(0xffff), HEX);

  if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
    Serial.println("yhdistetty!");
    failedAttempts = 0;
    lahetaDiscovery();
  } else {
    Serial.print("virhe, rc=");
    Serial.print(client.state());
    Serial.println("; jatketaan anturin ja LEDin toimintaa.");
    ++failedAttempts;
    if (failedAttempts >= 10) {
      Serial.println("MQTT: 10 yritystä epäonnistui; yritykset lopetetaan uudelleenkäynnistykseen asti.");
    }
  }
  lastAttempt = millis();
}
