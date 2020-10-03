#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient
// https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
#include <Adafruit_BME280.h> //https://github.com/adafruit/Adafruit_BME280_Library
#include "MHZ19.h" //https://github.com/WifWaf/MH-Z19

// Serial
#define SERIAL_BAUD 115200
#define LED_G 12
#define LED_B 13
#define LED_R 14

// Deep Sleep
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
RTC_DATA_ATTR int bootCount = 0;
long deepsleep = 0;             /* nomber of seconds to sleep after task completed 0 = no deepsleep*/

//Scheduling in loop
unsigned long  now = 0;
unsigned long  loopStart = 0;
unsigned short max_awakeTime = 5000;
unsigned short mqtt_poll_interval = 100;
unsigned long  mqtt_push_interval = 10000;
unsigned short vol_poll_interval = 100;


// Wifi and MQTT
#include "certs.h"
const char* mqtt_server = "mqtt.fadenstrahl.de";
const int   mqtt_server_port = 8883;
String clientId = "IGSE-ESP32-";
WiFiClientSecure secureSocket;
PubSubClient mqttClient(secureSocket);
#define _sub_topic_deepsleep "igs/environment/deepsleep"
#define _pub_topic_co2    "igs/environment/room1/co2"
#define _pub_topic_temp   "igs/environment/room1/temp"
#define _pub_topic_hum    "igs/environment/room1/hum"
#define _pub_topic_pres   "igs/environment/room1/pres"
#define _pub_topic_vol    "igs/environment/room1/vol"


//MHZ-19 Co2-Sensor
MHZ19 myMHZ19;                // Constructor for library
HardwareSerial mySerial(2);   // (ESP32 Example) create device to MH-Z19 serial
#define BAUDRATE 9600         // Device to MH-Z19 Serial baudrate (should not be changed)

//MAX9814 Microphone amp
#define MicPin 34
#define sampleWindow 50 // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
unsigned int maxVolume = 0; // max value during the mqtt_push_interval

// BME280
Adafruit_BME280 bme; // use I2C interface
Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();
Adafruit_Sensor *bme_pressure = bme.getPressureSensor();
Adafruit_Sensor *bme_humidity = bme.getHumiditySensor();


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  
  WiFi.begin(ssid, password);

  byte TryCounter = 0;
  while ((WiFi.status() != WL_CONNECTED) && (TryCounter < 10)) {
    delay(1000);
    TryCounter++;
    Serial.print(".");
  }
  if ((WiFi.status() != WL_CONNECTED) && (TryCounter > 9))
    ESP.restart();

  randomSeed(micros());

  Serial.print("\nConnected. IP: ");
  Serial.println(WiFi.localIP()); 
}


void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == _sub_topic_deepsleep) {
    deepsleep = messageTemp.toInt();
    if (deepsleep) {
      esp_sleep_enable_timer_wakeup(deepsleep * uS_TO_S_FACTOR);
      Serial.println("Setup ESP32 to sleep for every " + messageTemp + " Seconds");
    } else {
      esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
      Serial.println("Setup ESP32 to not perform any deepsleep.");
    }
    delay(500);
  } 
}


void reconnect_mqtt() {
  byte TryCounter = 0;
  // Loop until we're connected
  while (!mqttClient.connected() && (TryCounter < 10))
  {
    TryCounter++;

    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      
      // Subscribe topics
      if(mqttClient.subscribe(_sub_topic_deepsleep)){
        Serial.print("Subscribed to ");
      } else {
        Serial.print("Failed to subscribe to ");
      }
      Serial.println(_sub_topic_deepsleep);
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  if (!mqttClient.connected() && (TryCounter > 9))
    ESP.restart();
}


void publishMessage(const char* topic, const char* message, bool retain) {
  Serial.print("Message sent to topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  Serial.println(message);
  mqttClient.publish(topic, message, retain);
}


void measureCO2(){
  int CO2; 
  int8_t Temp;

  /* note: getCO2() default is command "CO2 Unlimited". This returns the correct CO2 reading even 
  if below background CO2 levels or above range (useful to validate sensor). You can use the 
  usual documented command with getCO2(false) */

  CO2 = myMHZ19.getCO2();                             // Request CO2 (as ppm)
  Temp = myMHZ19.getTemperature();                     // Request Temperature (as Celsius)

  if (CO2 < 1000)
  {
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, LOW);
  } else if (CO2 < 2000) {
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, LOW);    
  } else {
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, LOW);
  }

  //Serial.println("CO2 (ppm): " + String(CO2));
  //Serial.println("Temperature (C): " + String(Temp));
  
  publishMessage(_pub_topic_co2, String(CO2).c_str(), true);
  //publishMessage(_pub_topic_temp, String(Temp).c_str(), true);
}


unsigned int measureBME280(){
  sensors_event_t temp_event, pressure_event, humidity_event;
  bme_temp->getEvent(&temp_event);
  bme_pressure->getEvent(&pressure_event);
  bme_humidity->getEvent(&humidity_event);

  /*
  Serial.print(F("Temperature = "));
  Serial.print(temp_event.temperature);
  Serial.println(" *C");
  
  Serial.print(F("Humidity = "));
  Serial.print(humidity_event.relative_humidity);
  Serial.println(" %");

  Serial.print(F("Pressure = "));
  Serial.print(pressure_event.pressure);
  Serial.println(" hPa");
  */
  publishMessage(_pub_topic_temp, String(temp_event.temperature).c_str(), true);
  publishMessage(_pub_topic_hum,  String(humidity_event.relative_humidity).c_str(), true);
  publishMessage(_pub_topic_pres, String(pressure_event.pressure).c_str(), true);
}


unsigned int measureVolume(){
  unsigned long startMillis= millis();  // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level
  
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  // collect data for sampleWindow mS
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(MicPin);

    if (sample < 1024)  // toss out spurious readings
    {
      //if (sample > signalMax)
      signalMax = max(signalMax, sample);  // save just the max levels
      //else if (sample < signalMin)
      signalMin = min(signalMin, sample);  // save just the min levels
    }
  }
  if (!(signalMin == 1024 || signalMax == 0))
  {
    peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
    peakToPeak = map(peakToPeak, 0, 1023, 0 , 99);
  }

  return peakToPeak;
}


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  
  setup_wifi();
 
  secureSocket.setCACert(test_root_ca);
  mqttClient.setServer(mqtt_server, mqtt_server_port);
  mqttClient.setCallback(callback);

  reconnect_mqtt();

  mySerial.begin(BAUDRATE); 
  myMHZ19.begin(mySerial);                                // *Serial(Stream) refence must be passed to library begin(). 
  myMHZ19.autoCalibration();                              // Turn auto calibration ON (OFF autoCalibration(false))

  unsigned status = bme.begin(0x76);
  if (!status)
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
  
  //bme_temp->printSensorDetails();
  //bme_pressure->printSensorDetails();
  //bme_humidity->printSensorDetails();

  loopStart = millis();
}


void loop() {
  now = millis() - loopStart;

  if (now % mqtt_poll_interval == 0) 
  {
    if (!mqttClient.connected())
      reconnect_mqtt();
    mqttClient.loop();
  }
  
  if (now % vol_poll_interval == 0)
  {
    maxVolume = max(measureVolume(), maxVolume);
  }
  
  if (now % mqtt_push_interval == 0)
  {
    measureCO2();
    measureBME280();
    
    //Serial.println("Volume: " + String(maxVolume));
    publishMessage(_pub_topic_vol, String(maxVolume).c_str(), true);
    maxVolume = 0;
  }
  
  if (now > max_awakeTime && deepsleep) {
    mqttClient.disconnect();
    Serial.println("Going to sleep now");
    Serial.flush();
    esp_deep_sleep_start();
  }
  
}
