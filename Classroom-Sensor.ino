// Serial and IO
#define SERIAL_BAUD 115200
#define LED1B 12
#define LED1G 14
#define LED1R 27
#define LED2B 25
#define LED2G 33
#define LED2R 32
#define BUZZER 26
#define BTN1 35
#define BTN2 34


// Scheduling in loop and Deep Sleep
RTC_DATA_ATTR int bootCount = 0;
unsigned long deepsleep = 0; // Seconds to sleep after task completed 0 = no deepsleep
unsigned short max_awakeTime = 5; // Seconds to stay wake before next sleep
unsigned long  now = 0;
unsigned long  loopStart = 0;
unsigned short mqtt_poll_interval = 100;
unsigned long  mqtt_push_interval = 30000;
unsigned short vol_push_interval = 5000;

// Wifi
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "certs_p.h"
WiFiClientSecure secureSocket;

// MQTT
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient
// https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/
const char* mqtt_server = "mqtt.fadenstrahl.de";
const int   mqtt_server_port = 8883;
String clientId = "IGSE-ESP32-";
PubSubClient mqttClient(secureSocket);
#define _sub_topic_deepsleep "igs/environment/deepsleep"
#define _pub_topic_co2    "igs/environment/room1/co2"
#define _pub_topic_temp   "igs/environment/room1/temp"
#define _pub_topic_hum    "igs/environment/room1/hum"
#define _pub_topic_pres   "igs/environment/room1/pres"
#define _pub_topic_gas    "igs/environment/room1/gasresistance"
#define _pub_topic_vol    "igs/environment/room1/vol"

//MHZ-19 Co2-Sensor
#include "MHZ19.h"            //https://github.com/WifWaf/MH-Z19
MHZ19 myMHZ19;                // Constructor for library
HardwareSerial mySerial(2);   // (ESP32 Example) create device to MH-Z19 serial
#define BAUDRATE 9600         // Device to MH-Z19 Serial baudrate (should not be changed)

// BME680
#include "Zanshin_BME680.h"  // Include the BME680 Sensor library
BME680_Class BME680;  ///< Create an instance of the BME680 class

// INMP441
#include "slm.h"


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
      esp_sleep_enable_timer_wakeup(deepsleep * 1000000); // sleeptime in microseconds
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

  CO2 = myMHZ19.getCO2();             // Request CO2 (as ppm)
  Temp = myMHZ19.getTemperature();    // Request Temperature (as Celsius)

  if (CO2 < 1000)
  {
    digitalWrite(LED2R, LOW);
    digitalWrite(LED2G, LOW);
    digitalWrite(LED2B, LOW);
  } else if (CO2 < 1500) {
    digitalWrite(LED2R, LOW);
    digitalWrite(LED2G, LOW);
    digitalWrite(LED2B, HIGH);    
  } else if (CO2 < 2000) {
    digitalWrite(LED2R, LOW);
    digitalWrite(LED2G, HIGH);
    digitalWrite(LED2B, LOW);    
  } else {
    digitalWrite(LED2R, HIGH);
    digitalWrite(LED2G, LOW);
    digitalWrite(LED2B, LOW);
  }

  //Serial.println("CO2 (ppm): " + String(CO2));
  //Serial.println("Temperature (C): " + String(Temp));

  if (CO2 != 0 && CO2 != 5000)
  {
    publishMessage(_pub_topic_co2, String(CO2).c_str(), true);
    //publishMessage(_pub_topic_temp, String(Temp).c_str(), true);
  }
}

void measureBME680() {
  static int32_t  temp, humidity, pressure, gas;  // BME readings
  BME680.getSensorData(temp, humidity, pressure, gas);   // Get readings

  //Serial.print(F("Temperature = "));
  //Serial.print(float(temp)/100);
  //Serial.println(" \xC2\xB0\x43");
  publishMessage(_pub_topic_temp, String(float(temp)/100).c_str(), true);

  //Serial.print(F("Humidity = "));
  //Serial.print(float(humidity)/1000);
  //Serial.println(" %");
  publishMessage(_pub_topic_hum,  String(float(humidity)/1000).c_str(), true);
  
  //Serial.print(F("Pressure = "));
  //Serial.print(float(pressure)/100);
  //Serial.println(" hPa");
  publishMessage(_pub_topic_pres, String(float(pressure)/100).c_str(), true);
  
  //Serial.print(F("Gas Resistance = "));
  //Serial.print(float(gas)/100);
  //Serial.println(F(" m\xE2\x84\xA6"));
  publishMessage(_pub_topic_gas, String(float(gas)/100).c_str(), true);

  //const float seaLevel = 1013.25;
  //float altitude = 44330.0 * (1.0 - pow(((float)pressure / 100.0) / seaLevel, 0.1903));  // Convert into meters
  //Serial.print(F("Altitude = "));
  //Serial.print(altitude);
  //Serial.println(" m");
}


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  pinMode(LED1R, OUTPUT);
  pinMode(LED1G, OUTPUT);
  pinMode(LED1B, OUTPUT);
  pinMode(LED2R, OUTPUT);
  pinMode(LED2G, OUTPUT);
  pinMode(LED2B, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);

  ++bootCount;
  Serial.println("- Boot number: " + String(bootCount));

  Serial.print(F("- Initializing INMP441 Microphone\n"));
  samples_queue = xQueueCreate(8, sizeof(sum_queue_t));
  xTaskCreate(mic_i2s_reader_task, "Mic I2S Reader", I2S_TASK_STACK, NULL, I2S_TASK_PRI, NULL);

  Serial.print(F("- Initializing MHZ-19 CO\xE2\x82\x82 sensor\n")); //  "CO₂" symbol
  mySerial.begin(BAUDRATE); 
  myMHZ19.begin(mySerial);                                // *Serial(Stream) refence must be passed to library begin(). 
  myMHZ19.autoCalibration();                              // Turn auto calibration ON (OFF autoCalibration(false))

  Serial.println("- Initializing BME680 sensor");
  if (!BME680.begin(I2C_STANDARD_MODE)) // Start BME680 using I2C, use first device found
    Serial.println("Could not find a valid BME680 sensor, check wiring, address, sensor ID!");
  
  //Serial.print(F("- Setting 16x oversampling for all sensors\n"));
  BME680.setOversampling(TemperatureSensor, Oversample16);  // Use enumerated type values
  BME680.setOversampling(HumiditySensor, Oversample16);     // Use enumerated type values
  BME680.setOversampling(PressureSensor, Oversample16);     // Use enumerated type values
  //Serial.print(F("- Setting IIR filter to a value of 4 samples\n"));
  BME680.setIIRFilter(IIR4);  // Use enumerated type values
  //Serial.print(F("- Setting gas measurement to 320\xC2\xB0\x43 for 150ms\n"));  // "°C" symbols
  BME680.setGas(320, 150);  // 320°C for 150 milliseconds
  
  setup_wifi();
 
  secureSocket.setCACert(test_root_ca);
  mqttClient.setServer(mqtt_server, mqtt_server_port);
  mqttClient.setCallback(callback);

  reconnect_mqtt();

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
  
  if (now % vol_push_interval == 0)
  {
    double _Leq_dB;
    if(xQueueReceive(samples_queue, &_Leq_dB, portMAX_DELAY)){
        String tempString = String(_Leq_dB);
        //Serial.println(tempString);
        publishMessage(_pub_topic_vol, String(_Leq_dB).c_str(), true);
    }
  }
  
  if (now % mqtt_push_interval == 0)
  {
    measureCO2();
    measureBME680();
  }
  
  if (now > (max_awakeTime * 1000) && deepsleep) {
    mqttClient.disconnect();
    Serial.println("Going to sleep now");
    Serial.flush();
    esp_deep_sleep_start();
  }
  
}
