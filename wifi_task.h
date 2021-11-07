#include <WiFiClientSecure.h>
WiFiClientSecure secureSocket;

unsigned long wifi_reconnect_interval = 5000;

// FreeRTOS Task
static TaskHandle_t task_wifi;
#define WIFI_TASK_PRI   4
#define WIFI_TASK_STACK (1024 * 4)



void connect_wifi(void) {
  if (WiFi.status() != WL_CONNECTED) {  
    WiFi.begin(ssid, password);
    Serial.print(": Try connecting to ");
    Serial.println(ssid);
    delay( 3000 );
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("+ Wifi connection established! IP: ");
      Serial.println(WiFi.localIP());
    }
  }
}


void wifi_task(void* parameter) {
  String taskMessage = "+ Wifi-Task running on core ";
  taskMessage = taskMessage + xPortGetCoreID();
  Serial.println(taskMessage);
  
  for( ;; ) {
    //Serial.println(": Wifi Remaining Stack " + String(uxTaskGetStackHighWaterMark( NULL )));
    connect_wifi();
    delay( wifi_reconnect_interval );
  }  
}
