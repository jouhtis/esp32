# Kytkentäkuvat

## Liitäntäparit omilla laudoilla

![BMP280:n ja ESP32-S3:n merkityt liitäntäparit](kytkennat-omilla-laudoilla.png)

[Avaa vektorikuva (SVG)](kytkennat-omilla-laudoilla.svg).

Kuva on otettu ennen johdotusta. Värilliset numerot osoittavat liitäntäparit.

## Kytkentäkaavio

![ESP32-S3 ja BMP280](kytkentakaavio.svg)

BMP280: 3V3, GND, SDA GPIO 8 ja SCL GPIO 9. Laudan oma RGB-LED: GPIO 48.
Tarkista oman laudan pinout sekä anturilevyn CSB-, SDO- ja vetovastuskytkennät.
Kuvat eivät varmista valmiin johdotuksen oikeellisuutta.
