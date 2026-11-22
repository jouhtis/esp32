# Firmware operation

[Suomeksi](TOIMINTA_fi.md)

## Startup and LED

At startup, the firmware runs a roughly 14-second LED demo. The simulated
temperature rises from 29 °C to 40 °C, the LED flashes red three times,
the temperature drops to 30 °C, and the LED flashes blue three times.
The LED then turns off for 500 ms. The demo uses the normal color scale.
The firmware publishes only actual measurements over MQTT.

After the demo, the firmware initializes the BMP280 and sets the LED
according to the sensor reading.

| Temperature | LED |
|-------------|-----|
| Below 30 °C | Off |
| 30–40 °C | Blue through green to red |
| Above 40 °C | Red |

The NeoPixel uses GPIO 48. Brightness is 20, HSV saturation is 255, and
HSV value is 100. The BMP280 uses GPIO 8 for SDA and GPIO 9 for SCL.
The firmware checks I²C addresses `0x76` and `0x77` in that order.

## Network connections

The firmware reads Wi-Fi settings and MQTT credentials from the local
`env.h` file. It waits for Wi-Fi at startup, then starts NTP time
synchronization.

MQTT uses port 1883 without TLS. The firmware makes the first connection
attempt immediately and waits one minute after each failure. After ten
consecutive failures, it stops retrying until the device restarts.
A successful connection resets the counter. The firmware attempts MQTT
only while Wi-Fi is connected.

The sensor and LED continue to update between attempts. Each connection
attempt is synchronous, with a 250 ms TCP connection timeout and a
one-second MQTT response timeout. DNS resolution can add to the wait.

## Measurements and Home Assistant

The serial log runs at 115200 baud and prints the temperature every five
seconds. The firmware publishes an MQTT state message every 30 seconds
once MQTT is connected and NTP time is available. The topic is
`sensors/bmp280_<mac>/state`. The schema 2 JSON contains the per-device
`sensor_id`, the generic indoor `location` value `sisatila`, and the
temperature at `values.temperature`. State messages are non-retained.

The firmware derives `bmp280_<mac>` from the ESP32's factory MAC address.
The MAC part consists of 12 lowercase hexadecimal digits without
separators, for example `bmp280_001122aabbcc`. The MQTT client ID,
`sensor_id`, Home Assistant `unique_id`, and discovery device identifier
all use this value. It stays the same if the IP address changes. If the
firmware cannot read the factory MAC address, the sensor and LED keep
running and MQTT publishing is disabled.

After connecting to MQTT, the firmware publishes a retained discovery
message to `homeassistant/sensor/bmp280_<mac>/config`. The default device
and sensor names shown in Home Assistant are **BMP280-mittari**
(BMP280 thermometer) and **Lämpötila** (Temperature).

<a name="uuden-mittarin-käyttöönotto"></a>

### Adding a thermometer

1. Enter your Wi-Fi and MQTT settings in the local `env.h` file and make
   sure the Home Assistant MQTT integration uses the same broker.
2. Flash the `esp32s3usbotg` firmware to a compatible ESP32-S3 board.
   Boards with the same Wi-Fi and MQTT settings can use the same binary.
3. After startup, open the MQTT integration in Home Assistant. Select the
   new **BMP280-mittari** device, rename it, and assign it to an area.
   The sensor inherits the device's area.
4. Verify that the new sensor receives temperature readings and that
   existing thermometers still update their own entities.

### Upgrading from older firmware

Older firmware used the shared discovery topic
`homeassistant/sensor/esp32_temp/config`. After the upgrade, Home
Assistant discovers the MAC-based identifier as a new device. Confirm
that the new sensor receives a reading first. If the old discovery
configuration is still retained on the broker, clear it by publishing
an empty retained message to the old discovery topic.

Restore the device name and area as needed. Once the previous
`entity_id` is available, you can assign it to the new sensor to keep
existing dashboard and automation references working. Check that earlier
temperature readings are also available under that ID.

## Known limitations

The firmware waits for Wi-Fi before it starts taking continuous readings.
If the BMP280 is missing, the firmware continues running and may publish
an invalid reading. Reconnection and the ten-attempt limit still need
hardware verification, as does running two physical thermometers at
the same time.

[Wiring guide](docs/README.md) · [Enclosure model](cad/README.md)
