#include <MQTT.h>
MQTTClient mqtt_client(512);

String clientId = "IGSE-ESP32-";
String _PRE_TOPIC = "igs/environment/";
String room = "room262";

unsigned long mqtt_poll_interval = 500;

// FreeRTOS Queue
QueueHandle_t mqtt_queue = NULL;
#define MQTT_XQUEUE_SIZE 1

// FreeRTOS Task
static TaskHandle_t task_mqtt;
#define MQTT_TASK_PRI   4
#define MQTT_TASK_STACK 5000

struct mqtt_msg_t {
  String topic;
  String message;
  bool retain;
};



void connect_mqtt(void) {
  if ((WiFi.status() == WL_CONNECTED) && !mqtt_client.connected())
  {

    Serial.println(": Attempting MQTT connection");
    // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    
    if (mqtt_client.connect(clientId.c_str())) {
      Serial.println("+ MQTT connected");

      // Subscribe topics
      if (mqtt_client.subscribe(_PRE_TOPIC + "deepsleep")) {
        Serial.print("+ Subscribed to ");
      } else {
        Serial.print("- Failed to subscribe to ");
      }
      Serial.println(_PRE_TOPIC + "deepsleep");

    }    
  }
}


void messageReceived(String &topic, String &payload) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  Serial.println();

  /*
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
  */
}


void publishMessage(String topic, String message, bool retain) {
  Serial.print("Message sent to topic: ");
  Serial.print(_PRE_TOPIC + room + "/" + topic);
  Serial.print(". Message: ");
  Serial.println(message);

  if ((WiFi.status() == WL_CONNECTED) && mqtt_client.connected()) {
    mqtt_client.publish(_PRE_TOPIC + room + "/" + topic, message);
  }
}


void mqtt_task(void* parameter) {
  String taskMessage = "+ MQTT-Task running on core ";
  taskMessage = taskMessage + xPortGetCoreID();
  Serial.println(taskMessage);
  
  mqtt_client.begin(host, port, secureSocket);
  mqtt_client.onMessage(messageReceived);
  //vTaskDelay( 5000 / portTICK_PERIOD_MS );
  
  for( ;; ) {
    //Serial.println(": MQTT Remaining Stack" + String(uxTaskGetStackHighWaterMark( NULL )));
    connect_mqtt();
    
    if ((WiFi.status() == WL_CONNECTED) && mqtt_client.connected()) {
      mqtt_client.loop();
    }

    mqtt_msg_t pub_msg;
    if (xQueueReceive(mqtt_queue, &pub_msg, 0) == pdPASS) {
      publishMessage(pub_msg.topic, pub_msg.message, pub_msg.retain);
    }
    delay( mqtt_poll_interval );
  }  
}
