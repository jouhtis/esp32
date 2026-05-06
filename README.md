# esp32

Hakemistoittain erilaisia ESP32-virityksiä, kokeiluja ja muistiinpanoja.

Käyttö omalla vastuulla :)

## Nykyinen laite

Projektissa käytössä oleva kehityslauta on geneerinen ESP32-S3 DevKit -tyylinen
N16R8-controlleri kahdella USB-C-liittimellä.

- MCU: ESP32-S3
- Muisti: N16R8 tarkoittaa yleensä 16 MB flash + 8 MB PSRAM
- USB-C COM -portti: erillinen USB-UART-silta
- USB-C USB -portti: ESP32-S3:n native USB / OTG / CDC / JTAG -puoli
- macOS:ssa käytössä oleva COM-portti: `/dev/cu.usbmodem5B8F0714901`
- USB-sarjapiiri tunnistui nimellä `USB Single Serial`, vendor `0x1A86`

PlatformIOssa upload ja monitorointi tehdään COM-portin kautta:

```ini
board = freenove_esp32_s3_wroom
board_build.flash_size = 16MB
board_upload.flash_size = 16MB
board_build.partitions = default_16MB.csv
board_build.arduino.memory_type = qio_opi
upload_protocol = esptool
upload_port = /dev/cu.usbmodem5B8F0714901
monitor_port = /dev/cu.usbmodem5B8F0714901
```

Firmware on asetettu käyttämään UART-serialia, ei ESP32-S3:n native USB CDC:tä:

```ini
-D ARDUINO_USB_MODE=0
-D ARDUINO_USB_CDC_ON_BOOT=0
-D ARDUINO_USB_DFU_ON_BOOT=0
-D ARDUINO_USB_MSC_ON_BOOT=0
```

Nykyinen PlatformIO-board on `freenove_esp32_s3_wroom`, koska se on lähellä
geneeristä ESP32-S3 WROOM -lautaa ja käyttää valmiiksi UART/COM-porttia
serialiin. Boardin oletus on N8R8, joten projektissa flash-koko on
ylikirjoitettu N16R8-laudalle 16 MB:ksi.

## Lämpötila-anturi

Lämpötilan mittaukseen käytetään BMP280-anturia Adafruitin BMP280-kirjastolla.
Anturi on I2C-väylässä:

- SDA: GPIO 8
- SCL: GPIO 9
- I2C-osoitteet joita kokeillaan: `0x76` ja `0x77`
- Kirjasto: `adafruit/Adafruit BMP280 Library`

Koodi alustaa I2C:n näin:

```cpp
Wire.begin(8, 9);
```

Lämpötila luetaan funktiolla:

```cpp
bmp.readTemperature();
```

BMP280 mittaa lämpötilan ja ilmanpaineen. Tässä projektissa käytetään vain
lämpötilaa, joka julkaistaan MQTT:lla Home Assistantiin aiheeseen
`home/sensor/esp32_temp/state`.

## Muut kytkennät

Laudan sisäinen RGB/NeoPixel LED on koodissa GPIO 48:ssa:

```cpp
#define LED_PIN 48
```

LED pysyy pimeänä alle 30 C lämpötilassa ja vaihtaa väriä 30-40 C välillä.
