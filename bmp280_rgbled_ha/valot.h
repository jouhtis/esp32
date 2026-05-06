#include <Adafruit_NeoPixel.h>

#define LED_PIN 48
#define NUM_LEDS 1
Adafruit_NeoPixel pixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setupLedi() {
  pixel.begin();
  pixel.setBrightness(20);
  pixel.show();
}

void paivitaLedi(float t) {
  if (t < 30.0) {
    // Alle 30: Pidetään ledi pimeänä
    pixel.setPixelColor(0, pixel.Color(0, 0, 0)); 
  } 
  else {
    // --- HSV-säätö ---
    uint8_t saturation = 255; 
    uint8_t value = 100;      

    // Lämpötila 30.0 - 40.0 skaalataan Hue-arvoksi.
    // 30C = Sininen (43690)
    // 40C = Punainen (0)
    long hue = map(constrain(t * 10, 300, 400), 300, 400, 43690, 0);

    pixel.setPixelColor(0, pixel.ColorHSV(hue, saturation, value));
  }
  pixel.show();
}