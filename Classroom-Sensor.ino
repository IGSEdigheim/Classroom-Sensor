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
unsigned long  mqtt_push_interval = 15000;
unsigned short vol_push_interval = 5000;

// Wifi
#include <WiFiClientSecure.h>
#include "cred_p.h"
WiFiClientSecure secureSocket;

// MQTT
#include "mqtt_helper.h"

//MHZ-19 CO₂-Sensor
#include "MHZ19.h"            //https://github.com/WifWaf/MH-Z19
MHZ19 myMHZ19;                // Constructor for library
HardwareSerial mySerial(2);   // (ESP32 Example) create device to MH-Z19 serial
#define BAUDRATE 9600         // Device to MH-Z19 Serial baudrate (should not be changed)

// BME680
#include "bme680_helper.h"

// INMP441
#include "sound_level_meter.h"



void connect_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);

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



void measureCO2() {
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
    publishMessage(_pub_topic_co2, String(CO2), true);
    //publishMessage(_pub_topic_temp, String(Temp), true);
  }
}



void measureBME680() {

  if (iaqSensor.run()) { // If new data is available
    publishMessage(_pub_topic_temp,   String(iaqSensor.temperature), true);
    publishMessage(_pub_topic_hum,    String(iaqSensor.humidity), true);
    publishMessage(_pub_topic_pres,   String(iaqSensor.pressure / 100.0), true);
    publishMessage(_pub_topic_gas,    String(iaqSensor.gasResistance / 1000.0), true);

    if (iaqSensor.iaqAccuracy > 0)
    {
      publishMessage(_pub_topic_iaqAcc, String(iaqSensor.iaqAccuracy), true);
      publishMessage(_pub_topic_iaq,    String(iaqSensor.iaq), true);
      publishMessage(_pub_topic_siaq,   String(iaqSensor.staticIaq), true);
      publishMessage(_pub_topic_CO2eq,  String(iaqSensor.co2Equivalent), true);
      publishMessage(_pub_topic_bVOCeq, String(iaqSensor.breathVocEquivalent), true);
    }
    /*
      String output = "";
      output += "\ntemp " + String(iaqSensor.temperature) + " °C";
      output += "\nhum " + String(iaqSensor.humidity) + " %";
      output += "\npres " + String(iaqSensor.pressure / 100.0) + " hPa";
      output += "\ngasRes " + String(iaqSensor.gasResistance / 1000.0) + " kΩ";
      output += "\niaq " + String(iaqSensor.iaq);
      output += "\niaqAccuracy " + String(iaqSensor.iaqAccuracy); // 3==BSEC calibrated successfully
      output += "\nstaticIaq " + String(iaqSensor.staticIaq);
      output += "\nco2Equivalent " + String(iaqSensor.co2Equivalent) + " ppm";
      output += "\nbreathVocEquivalent " + String(iaqSensor.breathVocEquivalent) + " ppm";
      Serial.println(output);
    */
  } else {
    checkIaqSensorStatus();
  }
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

  BaseType_t xReturned = NULL;

  Serial.print(F("- Initializing INMP441 Microphone... "));
  slm_queue = xQueueCreate(I2S_XQUEUE_SIZE, sizeof(double));
  xReturned = xTaskCreate(mic_i2s_reader_task, "Mic I2S Reader", I2S_TASK_STACK, NULL, I2S_TASK_PRI, &task_slm);
  if ( xReturned == pdPASS )
  {
    Serial.print(F(" Task created.\n"));
  } else {
    Serial.print(F(" Could not create Task!\n"));
  }

  Serial.print(F("- Initializing MHZ-19 CO\xE2\x82\x82 sensor\n")); //  "CO₂" symbol
  mySerial.begin(BAUDRATE);
  myMHZ19.begin(mySerial);                                // *Serial(Stream) refence must be passed to library begin().
  myMHZ19.autoCalibration();                              // Turn auto calibration ON (OFF autoCalibration(false))

  Serial.print(F("- Initializing BME680 Environmental Sensor\n"));
  bme_i2c_init();

  WiFi.begin(ssid, password);
  connect_wifi();

  mqtt_client.begin(host, port, secureSocket);
  mqtt_client.onMessage(messageReceived);

  connect_mqtt();

  loopStart = millis();
}



void loop() {
  now = millis() - loopStart;

  if (now % mqtt_poll_interval == 0)
  {
    if (!mqtt_client.connected())
      connect_mqtt();
    mqtt_client.loop();
  }

  if (now % vol_push_interval == 0)
  {
    double _Leq_dB;
    if (xQueueReceive(slm_queue, &_Leq_dB, portMAX_DELAY)) {
      String tempString = String(_Leq_dB);
      //Serial.println(String(uxQueueMessagesWaiting(slm_queue)));
      //Serial.println(String(uxQueueSpacesAvailable(slm_queue)));

      publishMessage(_pub_topic_vol, String(_Leq_dB), true);
    }
  }

  if (now % mqtt_push_interval == 0)
  {
    measureCO2();
    measureBME680();
  }

  if (now > (max_awakeTime * 1000) && deepsleep) {
    mqtt_client.disconnect();
    Serial.println("Going to sleep now");
    Serial.flush();
    esp_deep_sleep_start();
  }

}
