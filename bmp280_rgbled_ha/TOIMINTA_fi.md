# Firmwaren toiminta

[English](TOIMINTA.md)

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
saatavilla. Aihe on `sensors/bmp280_<mac>/state`. Schema 2 -JSON sisältää
laitekohtaisen `sensor_id`:n, yleisen `location`-luokituksen `sisatila` ja
lämpötilan kentässä `values.temperature`. Tilaviestiä ei säilytetä brokerilla.

Firmware muodostaa tunnisteen `bmp280_<mac>` ESP32:n tehdas-MAC-osoitteesta.
MAC-osa koostuu 12 pienestä heksamerkistä ilman erottimia, esimerkiksi
`bmp280_001122aabbcc`. MQTT-asiakastunnus, `sensor_id`, Home Assistantin
`unique_id` ja discovery-laitetunniste käyttävät samaa arvoa. IP-osoitteen
vaihtuminen ei vaikuta tunnisteeseen. Jos tehdas-MAC:n luku epäonnistuu,
anturi ja LED toimivat edelleen ja MQTT-julkaisu jää pois.

Onnistuneen MQTT-yhdistämisen jälkeen firmware julkaisee säilytetyn
discovery-viestin aiheeseen `homeassistant/sensor/bmp280_<mac>/config`.
Home Assistantin oletusnimi laitteelle on **BMP280-mittari** ja anturille
**Lämpötila**.

### Uuden mittarin käyttöönotto

1. Täytä Wi-Fi- ja MQTT-asetukset paikalliseen `env.h`:hon ja varmista,
   että Home Assistantin MQTT-integraatio on yhdistetty samaan brokeriin.
2. Lataa `esp32s3usbotg`-firmware yhteensopivaan ESP32-S3-lautaan. Kun
   Wi-Fi- ja MQTT-asetukset ovat yhteiset, voit käyttää samaa binääriä
   kaikilla laudoilla.
3. Kun mittari on käynnistynyt, avaa Home Assistantin MQTT-integraatio.
   Valitse uusi **BMP280-mittari**, nimeä se ja määritä alue. Anturi perii
   alueen laitteelta.
4. Varmista, että uuden anturin lämpötilalukema päivittyy ja aiemmat
   mittarit lähettävät edelleen omiin entiteetteihinsä.

### Päivitys vanhasta firmwaresta

Vanha firmware käytti yhteistä discovery-aihetta
`homeassistant/sensor/esp32_temp/config`. Päivityksen jälkeen MAC-pohjainen
tunniste näkyy Home Assistantissa uutena laitteena. Tarkista ensin, että
uusi anturi saa mittauslukeman. Jos vanha discovery-määritys jää brokeriin,
poista se lähettämällä vanhaan discovery-aiheeseen tyhjä säilytetty viesti.
Palauta laitteen nimi ja alue tarvittaessa. Voit antaa uudelle anturille
vanhan `entity_id`:n, kun se on vapautunut, jotta aiemmat kojelauta- ja
automaatioviittaukset toimivat. Tarkista myös, että aiempi mittaushistoria
näkyy samalla tunnuksella.

## Tunnetut rajoitukset

Wi-Fi-yhteyden odotus estää jatkuvan mittaamisen käynnistyksessä.
Jos BMP280 ei löydy, ohjelma jatkaa ja voi julkaista virheellisen lukeman.
MQTT-yhteyden palautumista ja kymmenen yrityksen rajaa ei ole vielä
varmistettu laitteella.
Kahden fyysisen mittarin yhtäaikainen käyttö on vielä testaamatta.

[Kytkentäkuvat](docs/README_fi.md) · [Kotelomalli](cad/README_fi.md)
