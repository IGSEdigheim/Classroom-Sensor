# Classroom-Sensor

Der Classroom-Sensor verwendet einen ESP32-Mikrocontroller mit integriertem wifi-Modul. 

Durch den Einsatz von FreeRTOS Tasks werden die einzelnen Bestandteile des Programms entkoppelt.
Der Wifi-Task zum Beispiel, ist dafür zuständig, die wifi-Verbindung neu aufzubauen, falls sie abgebrochen wurde. Der Rest des Programms funktioniert aber mit und ohne wifi.

Mit den drei Sensoren werden die folgenden Umweltwerte gemessen
* CO₂-Konzentration mit dem MH-Z19
* Lufttemperatur, Luftfeuchtigkeit, Luftdruck und Luftwiderstand mit dem Bosch BME680
* Lautstärke in Dezibel mit dem INMP441-Mikrofon

Die beiden RGB-Leuchtdioden zeigen die Lautstärke und den CO2-Wert optisch an. Ein Summer kann zusätzlich ein akkustisches Feedback geben. Die beiden Taster können noch frei konfiguriert werden.

Alle Teile des Programms kommunizieren über FreeRTOS-Queues miteinander und funktionieren dadurch vollkommen asynchron. Der Button-Task sendet zum Beispiel eine Nachricht wenn ein Taster gedrückt oder losgelassen wurde. 

Die Platine wird in einem IKEA Bilderrahmen als Gehäuse untergebracht. Die "Frontblende" kann frei gestaltet werden. Dabei werden die LEDs integriert.

[Hier](https://grafana.fadenstrahl.de/d/bWRdm1dMk/environment?orgId=1&refresh=5s) können die Testdaten betrachtet werden.

In der Datei [cred.h](cred.h) müssen die WLAN-Zugangsdaten und URL/Port des MQTT-Servers eingetragen werden.

## ToDo

* Die Taster und der Summer funktionieren aber haben noch keine Funktion.

# Software

## Verwendete Arduino Bibliotheken

* [WifiClientSecure](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFiClientSecure) für TLS
* [arduino-mqtt](https://github.com/256dpi/arduino-mqtt) für das MQTT-Protokoll
* [MH-Z19](https://github.com/WifWaf/MH-Z19) für den CO₂-Sensor
* [BME680](https://github.com/Zanduino/BME680) für den BME680
* Aus [INMP441](https://github.com/ikostoski/esp32-i2s-slm) entstammt die [sound_level_meter.h](sound_level_meter.h) ursprünglich (Aus den Rohdaten des Mikrofons die Lautstärke zu berechnen ist anspruchsvoll).
* [jled](https://github.com/jandelgado/jled#static-on-and-off) um die LEDs einfach anzusteuern

## Datenfluss

![Datenfluss](img/Datenfluss.png)

## MQTT-Topics
* igs/environment/deepsleep
* igs/environment/RAUM_ID/co2
* igs/environment/RAUM_ID/bme680
* igs/environment/RAUM_ID/vol

Wobei **RAUM_ID** durch eine eindeutige Kennzeichnung des Raumes ersetzt wird. Zu Testzwecken verwende ich *raum1* und *raum2*.

# Hardware

## Platine

Auf der zugehörigen [Projektseite auf oshwlab.com](https://oshwlab.com/Classroom-Sensor/classroom-sensor) können Sie den Schaltplan und die Platine bearbeiten und von jlcpcb fertig lassen.


![3D-Ansicht](img/Classroom-Sensor_v2_3D.PNG)
![Platine](img/Classroom-Sensor_v2_2D.png)
![Schaltskizze](img/Classroom-Sensor_v2_Schematic.png)

* Widerstände:
R1 = 100, R2 = 120, R3 = 47, R4 = 100, R5 = 120, R6 = 47, R7 = 10k, R8 = 10k

* Außenmaß:
Die Innenmaße des [Ikea Rahmens Ribba 10x15](https://www.ikea.com/de/de/p/ribba-rahmen-weiss-50378410/) definieren den Umriss der Platine (152.5mm x 102.6mm).
Die Löcher (3,2mm) sind dafür da um optional den Feinstaubsensor auf die Rückseite der Platine anzuschrauben.

### Anschlussmöglichkeiten für die LEDs

Auf der Platine gibt es zwei Anschlussmöglichkeiten für bis zu 6 LEDs. Eine RGB-LED ersetzt 3 LEDs. Es lassen sich also zwei RGB-LEDs anschließen und beliebig programmieren.
Die erste LED könnte die CO₂-Konzentration und die zweite die Lautstärke anzeigen.

Die LEDs kann man an einem beliebigen Platz direkt auf die Platine auflöten indem man für eine RGB-LED vier nebeneinanderstehende freie Lötlöcher auswählt.
Alle Löcher mit quadratischer Lötstelle sind mit GND (Minus) verbunden. Die anderen Löcher (mit runden Lötstellen) sind nicht verbunden.
Für jede LED braucht es also eine Kabelverbindung zu einem der LED Anschlussstellen, die mit LED1 und LED2 beschriftet sind.

Alternativ kann man die LEDs auch mit einem Kabel mit der Platine verbinden, wenn man einen Connector auf die Platine lötet.
Es eignet sich zu Beispiel ein abgewinkelter [2X4P IDC Connector](https://de.aliexpress.com/item/4000113049761.html) oder ein 2x4 Pin Header.
LCSC: [2X4P IDC Connector](https://lcsc.com/product-detail/IDC-Connectors_BOOMELE-Boom-Precision-Elec-C9139_C9139.html) oder [C492432](https://lcsc.com/product-detail/Pin-Header-Female-Header_XFCN-PZ254R-12-8P_C492432.html)


### Anschluss für Feinstaubsensor (SDS011)

Dieser Anschluss ist für den Classroom-Sensor absolut optional.
Dabei Feinstaubsensor handelt es sich um ein total anderes Projekt, dass sich mit dieser Platine ebenfalls umsetzten lässt.

[Datenblatt des Feinstaubsensors](https://cdn-reichelt.de/documents/datenblatt/X200/SDS011-DATASHEET.pdf)

#### Bezugsquellen
* Sensor: [AliExpress](https://de.aliexpress.com/item/32606349048.html)
* Buchse: [AliExpress](https://www.aliexpress.com/item/4000029760504.html)
* LCSC [XH-5AW](https://lcsc.com/product-detail/Wire-To-Board-Wire-To-Wire-Connector_BOOMELE-Boom-Precision-Elec-XH-5AW_C24023.html)

## Mikrocontroller, Sensoren und Aktoren

### ESP32 (Doit 30Pin Version)
<img src="https://ae01.alicdn.com/kf/HTB1_cCCac_vK1RkSmRyq6xwupXaM.jpg" width="200">

Jeder Pin des ESP32 Entwicklungsboards ist auf der Platine herausgeführt.
Bei Bedarf können zwei [einreihige Pin-Buchsen](https://www.aliexpress.com/item/32970948352.html) (15 Pins pro Seite) aufgelötet werden.

#### Bezugsquelle
* [AliExpress](https://www.aliexpress.com/item/32959541446.html)

### CO₂-Sensor: (MH-Z19)
<img src="https://ae01.alicdn.com/kf/H21416e6fddfb46539fdf563d8bf5ec212.jpg" width="200">

#### Bezugquelle
* [AliExpress](https://www.aliexpress.com/item/4000212024923.html)

### Umweltsensor (BME680)
<img src="https://ae01.alicdn.com/kf/H9966c391bc334a649263d5f16d010e0dY.jpg" width="200">

#### Bezugsquelle
* [AliExpress](https://www.aliexpress.com/item/4000049700826.html)

### Mikrofon (INMP441)
<img src="https://ae01.alicdn.com/kf/Hb5534b3132464cae9076f58626cec9fdm.jpg" width="200">

Zur Lärmpegelmessung.

#### Bezugsquelle
* [AliExpress](https://www.aliexpress.com/item/32961274528.html)

### Summer (Buzzer)

#### Bezugsquellen
* [AliExpress](https://www.aliexpress.com/item/32416854447.html)
* LCSC: [SEA-12085-16](https://lcsc.com/product-detail/Buzzers_Made-in-China-SEA-12085-16_C2858.html)

### Taster

#### Bezugsquellen
* [AliExpress](https://de.aliexpress.com/item/4000800019378.html)
* LCSC: [SKHHNHA010](https://lcsc.com/product-detail/Tactile-Switches_ALPSALPINE-SKHHNHA010_C219779.html)

### RGB-LED

Man hat die Wahl zwischen zwei RGB Leuchtdioden mit *gemeinsamer Kathode* oder bis zu 6 einzelnen LEDs. Man kann auch eine RGB LED und 3 einzelne LEDs verwenden.
Ich rate zu diffusen LEDs, da sich die Farben besser mischen.

#### Bezugsquelle
* [AliExpress](https://de.aliexpress.com/item/32278313170.html)

### Strombuchse für 5V

#### Bezugsquelle
* [AliExpress](https://de.aliexpress.com/item/32730260523.html)

## Links zu weiteren Projekten

* https://randomnerdtutorials.com/build-an-all-in-one-esp32-weather-station-shield/ 
* https://github.com/kadamski/dust_sensor 
* https://www.umwelt-campus.de/forschung/projekte/iot-werkstatt/ideen-zur-corona-krise-1
* http://steinlaus.de/stinkt-das-hier-teil-1-mit-dem-bosch-bme680/
