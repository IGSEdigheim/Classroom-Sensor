# Classroom-Sensor
Sensor on ESP32 with CO2, MEMS

## Hardware

#### ESP32: [Doit 30Pin Version:](https://www.aliexpress.com/item/32959541446.html) <img src="https://ae01.alicdn.com/kf/HTB1_cCCac_vK1RkSmRyq6xwupXaM.jpg" width="200">

Jeder Pin sollte doppelt auf der Platine sein, so dass jeder Pin verfügbar ist wenn der ESP auf die PLatine gesteckt ist. Diese Buchsen finden Verwendung:
* Single: https://www.aliexpress.com/item/32970948352.html 
* Double: https://www.aliexpress.com/item/32956866217.html 

Sensoren / Aktoren

#### CO2-Sensor: [MH-Z19](https://www.aliexpress.com/item/4000212024923.html)
<img src="https://ae01.alicdn.com/kf/H21416e6fddfb46539fdf563d8bf5ec212.jpg" width="200">

#### Umweltsensor [BME680](https://www.aliexpress.com/item/4000049700826.html)
<img src="https://ae01.alicdn.com/kf/H9966c391bc334a649263d5f16d010e0dY.jpg" width="200">

#### Helligkeitssensor 

#### Mikrofon [INMP441](https://www.aliexpress.com/item/32961274528.html)
<img src="https://ae01.alicdn.com/kf/Hb5534b3132464cae9076f58626cec9fdm.jpg" width="200">
Zur Lärmpegelmessung. Der Eingang zum Mikrofon ist auf der Unterseite der Platine. 

#### Piezo-Summer 


### Anschlüsse 
Neben den Anschlüssen für die Sensoren kommen außerdem noch die folgenden Anschlüsse auf die Platine.

#### LED-Anschlüsse (6LEDs) 
Man kann eine RGB-LED oder 3 LEDs mit einem Dupont-Kabel direkt an die Platine anschließen. Das geht mit 7 Pins. (4mal GND)
Das Ganze zwei Mal. Also 14 Pins. 6 Für die Anoden und 8 mal GND

#### STEMMA-QT 
Um den Funktionsumfang zu erweitern. Stemma-QT ist ein neuer Standard von Adafruit für I2C mit 4 Pins.

#### Anschluss für Feinstaubsensor SDS011 
Auf der Platine sollte ein [5-Pin JST-XH Connector](https://www.aliexpress.com/item/4000029760504.html) für den Feinstaubsensor SDS011 am Rand vorhanden sein. 


### Die Platine

#### Außenmaße: 70x70 mm
Die Maße des Feinstaubsensors definieren den Umriss der Platine (70mm x 70mm). Die Löcher (3,2mm) an den richtigen Stellen einplanen damit man den Feinstaubsensor auf die Rückseite der Platine schrauben kann.  

[Datenblatt](https://cdn-reichelt.de/documents/datenblatt/X200/SDS011-DATASHEET.pdf)
 

### Andere Projekte im Netz

* https://randomnerdtutorials.com/build-an-all-in-one-esp32-weather-station-shield/ 
* https://github.com/kadamski/dust_sensor 
