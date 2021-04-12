#include <MQTT.h>
MQTTClient mqtt_client;
String clientId = "IGSE-ESP32-";
#define _sub_topic_deepsleep "igs/environment/deepsleep"
#define _pub_topic_co2    "igs/environment/room1/co2"
#define _pub_topic_vol    "igs/environment/room1/vol"

#define _pub_topic_temp   "igs/environment/room1/temp"
#define _pub_topic_hum    "igs/environment/room1/hum"
#define _pub_topic_pres   "igs/environment/room1/pres"
#define _pub_topic_gas    "igs/environment/room1/gasresistance"
#define _pub_topic_iaqAcc "igs/environment/room1/iaqaccuracy"
#define _pub_topic_iaq    "igs/environment/room1/iaq"
#define _pub_topic_siaq   "igs/environment/room1/siaq"
#define _pub_topic_CO2eq  "igs/environment/room1/co2eq"
#define _pub_topic_bVOCeq "igs/environment/room1/bvoceq"

void connect_mqtt() {
  byte TryCounter = 0;
  // Loop until we're connected
  while (!mqtt_client.connected() && (TryCounter < 10))
  {
    TryCounter++;

    Serial.print("Attempting MQTT connection");
    // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt_client.connect(clientId.c_str())) {
      Serial.println(" connected");

      // Subscribe topics
      if (mqtt_client.subscribe(_sub_topic_deepsleep)) {
        Serial.print("Subscribed to ");
      } else {
        Serial.print("Failed to subscribe to ");
      }
      Serial.println(_sub_topic_deepsleep);

    } else {
      Serial.print(".");
      delay(5000);
    }
  }
  if (!mqtt_client.connected() && (TryCounter > 9))
    ESP.restart();
}


void messageReceived(String &topic, String &payload) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  Serial.println();

  if (String(topic) == _sub_topic_deepsleep) {
    deepsleep = payload.toInt();
    if (deepsleep) {
      esp_sleep_enable_timer_wakeup(deepsleep * 1000000); // sleeptime in microseconds
      Serial.println("Setup ESP32 to sleep for every " + payload + " Seconds");
    } else {
      esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
      Serial.println("Setup ESP32 to not perform any deepsleep.");
    }
    delay(500);
  }
}

void publishMessage(String topic, String message, bool retain) {
  Serial.print("Message sent to topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  Serial.println(message);
  mqtt_client.publish(topic, message);
}
