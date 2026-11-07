# Prototyyppi – Toimintaa ja käytäntö (bmp280_rgbled_ha)

Tämä tiedosto on ajantasainen runtime-kontratti prototyypille
(`bmp280_rgbled_ha.ino`, `sensorit.h`, `valot.h` ja `wifi_mqtt.h`).

## I2C ja BMP280

- Anturi on I2C-väylässä: SDA GPIO 8, SCL GPIO 9 (`Wire.begin(8, 9)`).
- BMP280-kirjaston `begin()` kokee osoitteita `0x76` ja `0x77`.
- Lämpötila luetaan `bmp.readTemperature()`.

## LED

- Yksi NeoPixel, GPIO 48, brightness 20.
- Jokaisessa käynnistyksessä LED-demo ennen anturin ja verkkoyhteyksien
  alustusta: testilämpötila 29–40 °C ja takaisin 30 °C:een,
  0,1 °C askel / 50 ms. Punaisessa ja sinisessä päässä kolme vilkutusta
  (250 ms pois / 250 ms päällä), lopuksi LED pois 500 ms.
  Demo kestää noin 14 s ja käyttää `paivitaLedi()`-väriskaalaa.
  Testiarvoja ei julkaista MQTT:llä. Demon jälkeen anturi alustetaan ja LED
  asetetaan oikean mittauksen mukaan ennen Wi-Fi-yhteyden odottamista.
- LED pysyy pimeänä, kun lämpötila on alle 30 °C.
- Lämpötila 30–40 °C: hue kartanetaan 43690 (sininen) → 0 (punainen);
  saturaatio 255, HSV-arvo 100.
- Lämpötila rajoitetaan 30–40 °C:n välille.

## Fyysiset kytkennät ja 3D-malli

Tarkemmat osat, GPIO-virit, kytkentäkaavio sekä paikat kuville ja 3D-mallille:
`bmp280_rgbled_ha/docs/kytkennat.md`.

## Puuttuva anturi

- Jos BMP280 ei löydy, setup ei pysäytä koodia. Wi-Fi ja MQTT alustautuvat silti.
- loop() kutsuu edelleen `lueLampotila()` ja julkaisee vielä 30 s välein, joten
  puuttuvalla sensorilla voidaan julkaista huonoa lukua.

## Luku- ja julkausjärjestys

- Seriaportti 115200. Lämpötilatietue joka 5 s.
- MQTT-state joka 30 s.

## Wi-Fi ja MQTT

- `setupWiFi()` odottaa, että Wi-Fi yhdistyy (`WiFi.status() == WL_CONNECTED`).
- MQTT port 1883, ei TLS. `setupMQTT()` asettaa yhteysasetukset.
- `loop()` päivittää anturin ja LEDin ja kutsuu `yllapidaMQTT()`-funktiota.
  Se yrittää yhteyttä kerran ja palaa; seuraava yritys tehdään aikaisintaan
  60 s edellisen epäonnistuneen yrityksen päättymisestä. Ensimmäinen yritys
  tehdään heti. Wi-Fi-yhteyden puuttuessa ei yritetä eikä laskuri kasva.
- Enintään 10 peräkkäistä epäonnistunutta MQTT-yhteysyritystä. Sen jälkeen
  yritykset lopetetaan uudelleenkäynnistykseen asti. Onnistunut yhteys nollaa
  laskurin, joten myöhemmän yhteyskatkon jälkeen yrityksiä on jälleen 10.
- MQTT-katkos ei jätä ohjelmaa uudelleenyrityssilmukkaan. Yksittäinen yritys
  on edelleen synkroninen: TCP-yhteyden aikaraja 250 ms ja MQTT-vastauksen
  odotus 1 s. Nimipalvelun viive voi lisäksi pidentää yritystä.
- Tilaviestit julkaistaan vain MQTT-yhteyden ollessa auki. Discovery lähetetään
  jokaisen onnistuneen MQTT-yhdistämisen jälkeen kuten aiemminkin.
- Client id: `ESP32S3-Temp-<4 hex digits>`.

## Home Assistant -integraatio

- Tila: `sensors/bmp280_sisatila/state` – schema 2 -JSON, jossa lämpötila on
  polussa `values.temperature`; viesti ei ole retained.
- Discovery: `homeassistant/sensor/esp32_temp/config` – retained. Nimi
  "TV taso Lämpötila", stat_t tila, unit °C, device_class temperature, val_tpl
  `{{ value_json.values.temperature }}`, uniq_id `esp32s3_bmp280_01`, dev ids `["esp32s3_01"]`, device
  name "ESP32 S3 Mittari".
- PubSubClient buffer 512 bytes, koska discovery-payload ei maaliudu oletusbufferiin.

## Salaisuudet

- `WIFI_SSID`, `WIFI_PASSWORD`, `MQTT_SERVER`, `MQTT_USER`, `MQTT_PASSWORD`
  luetaan `env.h`:stä.
