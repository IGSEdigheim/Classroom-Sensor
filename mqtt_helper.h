#include <MQTT.h>
MQTTClient mqtt_client(512);

String clientId = "IGSE-ESP32-";
String _PRE_TOPIC = "igs/environment/";
String room = "room1";

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
      if (mqtt_client.subscribe(_PRE_TOPIC + "deepsleep")) {
        Serial.print("Subscribed to ");
      } else {
        Serial.print("Failed to subscribe to ");
      }
      Serial.println(_PRE_TOPIC + "deepsleep");

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

  if (String(topic) == _PRE_TOPIC + "deepsleep") {
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
  Serial.print(_PRE_TOPIC + room + "/" + topic);
  Serial.print(". Message: ");
  Serial.println(message);
  mqtt_client.publish(_PRE_TOPIC + room + "/" + topic, message);
}
