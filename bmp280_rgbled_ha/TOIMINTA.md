# Firmwaren toiminta

## Käynnistys ja LED

Käynnistyksessä ajetaan noin 14 sekunnin LED-demo: testilämpötila nousee
29 → 40 °C, punainen vilkkuu kolme kertaa, lämpötila laskee 30 °C:een
ja sininen vilkkuu kolme kertaa. Lopuksi LED sammuu 500 ms ajaksi.
Demo käyttää normaalia väriskaalaa, eikä testiarvoja julkaista MQTT:llä.

Demon jälkeen alustetaan BMP280 ja LED asetetaan anturin lukeman mukaan:

| Lämpötila | LED |
|-----------|-----|
| Alle 30 °C | Pois |
| 30–40 °C | Sinisestä vihreän kautta punaiseen |
| Yli 40 °C | Punainen |

NeoPixel on GPIO 48:ssa. Kirkkaus on 20, HSV-saturaatio 255 ja arvo 100.
BMP280 käyttää SDA GPIO 8:aa ja SCL GPIO 9:ää; osoitteet kokeillaan
järjestyksessä `0x76`, `0x77`.

## Verkkoyhteydet

Wi-Fi-asetukset ja MQTT-tunnukset luetaan paikallisesta `env.h`:stä.
Käynnistys odottaa Wi-Fi-yhteyttä, minkä jälkeen NTP-aika alustetaan.

MQTT käyttää porttia 1883 ilman TLS:ää. Ensimmäinen yhteysyritys tehdään
heti; epäonnistumisen jälkeen odotetaan minuutti. Kymmenen peräkkäisen
epäonnistumisen jälkeen yritykset loppuvat uudelleenkäynnistykseen asti.
Onnistunut yhteys nollaa laskurin. Ilman Wi-Fiä MQTT-yhteyttä ei yritetä.

Anturin ja LEDin päivitys jatkuu yritysten välillä. Yksittäinen yritys on
synkroninen: TCP-yhteyden aikaraja on 250 ms, MQTT-vastauksen 1 s.
Nimipalvelun viive voi pidentää odotusta.

## Mittaukset ja Home Assistant

Sarjaloki toimii nopeudella 115200 ja tulostaa lämpötilan 5 sekunnin välein.
MQTT-tilaviesti lähetetään 30 sekunnin välein, kun yhteys ja NTP-aika ovat
saatavilla. Aihe on `sensors/bmp280_sisatila/state`, sisältö schema 2 -JSON
ja lämpötila kentässä `values.temperature`. Viesti ei ole retained.

Home Assistant discovery lähetetään jokaisen onnistuneen yhdistämisen
jälkeen retained-viestinä aiheeseen `homeassistant/sensor/esp32_temp/config`.
Sensorin tunnus on `esp32s3_bmp280_01` ja laitteen `esp32s3_01`.
Vaihda tunnukset `wifi_mqtt.h`:ssa, jos käytät useampaa laitetta.

## Tunnetut rajoitukset

Wi-Fi-yhteyden odotus estää jatkuvan mittaamisen käynnistyksessä.
Jos BMP280 ei löydy, ohjelma jatkaa ja voi julkaista virheellisen lukeman.
MQTT-yhteyden palautumista ja kymmenen yrityksen rajaa ei ole vielä
varmistettu laitteella.

[Kytkentäkuvat](docs/README.md) · [Kotelomalli](cad/README.md)
