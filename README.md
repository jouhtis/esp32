# ESP32-S3 RGB-lämpömittari

ESP32-S3, BMP280 ja yksi NeoPixel: LED näyttää lämpötilan värinä ja
mittaukset julkaistaan MQTT:llä Home Assistantiin.

## Laitteisto

Nykyinen kokoonpano käyttää geneeristä ESP32-S3 N16R8 -lautaa.
PlatformIO-profiili on `esp32s3usbotg`, board `freenove_esp32_s3_wroom`,
16 MB flash ja QIO/OPI-muistiasetukset. Tarkista oman laudan yhteensopivuus.
BMP280:n I2C: SDA GPIO 8, SCL GPIO 9; osoitteet 0x76 ja 0x77.
NeoPixel: GPIO 48. Varmista oman laudan pinnit ja kytkennät ennen käyttöä.

## Käyttöönotto

1. Asenna VS Code ja PlatformIO IDE (`platformio.platformio-ide`).
2. Avaa tämä kansio VS Codessa (`platformio.ini` juurihakemistossa).
3. Kopioi `bmp280_rgbled_ha/env.example.h` tiedostoksi `env.h` samaan kansioon
   ja täytä Wi-Fi- ja MQTT-asetukset. Älä lisää tunnustiedostoa Gitiin.
4. Kytke USB-datakaapeli laudan COM-/USB-UART-liittimeen.
5. Avaa PlatformIO-kuvake ja **Project Tasks → esp32s3usbotg → General**.
   **Build** kääntää, **Upload** lataa ja **Monitor** näyttää sarjalokin.
   Sulje Monitor ennen lataamista. Sarjanopeus on 115200.

PlatformIO-terminalissa:

```sh
pio device list
pio run -e esp32s3usbotg
pio run -e esp32s3usbotg -t upload
pio device monitor -b 115200
```

Sarjaportti tunnistetaan automaattisesti. Jos laitteita on useita, määritä
oman koneesi `upload_port` ja `monitor_port` tiedostossa `platformio.ini`.
PlatformIO löytyy myös komentopaletista: **PlatformIO: Home**.
Devices näyttää portit, Inspect muistinkäytön ja Monitor laitteen tulosteet.

## Toiminta

Käynnistyksen noin 14 sekunnin demo käy väriskaalan läpi molempiin suuntiin
ja vilkuttaa punaista ja sinistä. Sen jälkeen käytetään oikeaa lämpötilaa:
alle 30 °C LED on pimeä; 30–40 °C väri muuttuu sinisestä punaiseen.
RESET-painike käynnistää demon uudestaan.

MQTT-yhteyttä yritetään heti ja epäonnistumisen jälkeen minuutin tauolla,
enintään 10 peräkkäistä epäonnistumista. Onnistuminen nollaa laskurin.
Anturi ja LED toimivat yritysten välillä. Yksittäinen synkroninen yritys
voi viivästyttää silmukkaa. Wi-Fi-yhteyden odotus käynnistyksessä on edelleen
estävää. Puuttuvan anturin käsittely on keskeneräinen.

MQTT käyttää porttia 1883 ilman TLS:ää. Tilaviesti on schema 2 -JSON
aiheessa `sensors/bmp280_sisatila/state`, lämpötila `values.temperature`.
Home Assistant discovery lähetetään retained-viestinä aiheeseen
`homeassistant/sensor/esp32_temp/config`. Vaihda laite- ja sensoritunnukset,
jos käytät useampaa laitetta. Julkaisu edellyttää NTP-aikaa.

Tarkempi toimintakuvaus: [TOIMINTA.md](bmp280_rgbled_ha/TOIMINTA.md).

## Tarkistukset

Firmware on käännetty PlatformIOlla ja ladattu ESP32-S3-laitteeseen.
RGB-demo on kokeiltu laitteella. MQTT-yhteyden palautumista ja kaikkia
kymmentä yhteysyritystä ei ole vielä testattu laitteella.
