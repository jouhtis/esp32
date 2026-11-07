#include <Wire.h>
#include <Adafruit_BMP280.h>

#define SDA_PIN 8
#define SCL_PIN 9
Adafruit_BMP280 bmp;

void setupBMP() {
  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("Etsitään BMP280-sensoria...");

  bool found = false;
  byte osoitteet[] = {0x76, 0x77}; // Kaksi yleisintä BMP280-osoitetta

  for (int i = 0; i < 2; i++) {
    Serial.print("Yritetään osoitetta: 0x");
    Serial.println(osoitteet[i], HEX);

    if (bmp.begin(osoitteet[i])) {
      Serial.print("BMP280 löytyi osoitteesta: 0x");
      Serial.println(osoitteet[i], HEX);
      found = true;
      break;
    }
  }

  if (!found) {
    Serial.println("!!! VIRHE: BMP280-sensoria ei löytynyt kummastakaan osoitteesta !!!");
    Serial.println("Tarkista SDA (Pin 8) ja SCL (Pin 9) kytkennät.");
    // Emme pysäytä koodia tähän (while(1)), jotta WiFi/MQTT voivat silti toimia
  }
}

float lueLampotila() {
  return bmp.readTemperature();
}
