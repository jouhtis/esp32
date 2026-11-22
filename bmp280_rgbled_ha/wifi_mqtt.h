#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_mac.h>
#include <time.h>
#include "env.h"

namespace MqttDevice {
constexpr char location[] = "sisatila";
constexpr char sensorName[] = "Lämpötila";
constexpr char deviceName[] = "BMP280-mittari";

char sensorId[sizeof("bmp280_000000000000")] = {};
String discoveryTopic;
String stateTopic;
}

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER;
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASSWORD;

WiFiClient espClient;
PubSubClient client(espClient);

bool utcTimestamp(char* output, size_t outputSize) {
  time_t now = time(nullptr);
  if (now < 1700000000) return false;
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  return strftime(output, outputSize, "%Y-%m-%dT%H:%M:%SZ", &timeinfo) > 0;
}

void lahetaDiscovery() {
  String payload = "{\"name\":\"";
  payload += MqttDevice::sensorName;
  payload += "\",\"stat_t\":\"";
  payload += MqttDevice::stateTopic;
  payload += "\",\"unit_of_meas\":\"°C\","
    "\"dev_cla\":\"temperature\","
    "\"val_tpl\":\"{{ value_json['values']['temperature'] }}\","
    "\"uniq_id\":\"";
  payload += MqttDevice::sensorId;
  payload += "\",\"dev\":{\"ids\":[\"";
  payload += MqttDevice::sensorId;
  payload += "\"],\"name\":\"";
  payload += MqttDevice::deviceName;
  payload += "\"}}";

  if (client.publish(MqttDevice::discoveryTopic.c_str(), payload.c_str(), true)) {
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
  uint8_t mac[6];
  if (esp_efuse_mac_get_default(mac) != ESP_OK) {
    Serial.println("MQTT-tunnisteen alustus epäonnistui: tehtaan MAC-osoitetta ei voitu lukea.");
    return;
  }
  snprintf(MqttDevice::sensorId, sizeof(MqttDevice::sensorId),
           "bmp280_%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  MqttDevice::stateTopic = String("sensors/") + MqttDevice::sensorId + "/state";
  MqttDevice::discoveryTopic = String("homeassistant/sensor/") + MqttDevice::sensorId + "/config";
  client.setServer(mqtt_server, 1883);
  client.setBufferSize(512);
  // Rajoita yksittäisen synkronisen yhteysyrityksen odotuksia.
  espClient.setConnectionTimeout(250); // TCP-yhteys, millisekuntia
  client.setSocketTimeout(1);          // MQTT-vastaus, sekuntia
}

void yllapidaMQTT() {
  if (MqttDevice::sensorId[0] == '\0' || WiFi.status() != WL_CONNECTED) return;
  if (client.connected()) {
    client.loop();
    return;
  }

  static uint8_t failedAttempts = 0;
  static unsigned long lastAttempt = 0;
  if (failedAttempts >= 10) return;
  if (failedAttempts > 0 && millis() - lastAttempt < 60000UL) return;

  Serial.print("Yhdistetään MQTT...");
  if (client.connect(MqttDevice::sensorId, mqtt_user, mqtt_pass)) {
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
