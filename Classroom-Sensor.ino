#include "cred.h"
#include "wifi_task.h"
#include "mqtt_task.h"
#include "slm_task.h"
#include "mhz19_task.h"
#include "bme_task.h"
#include "button_task.h"
#include "buzzer_task.h"
#include "led_task.h"


unsigned long VolumeDelay = 5000;
unsigned long lastVolumeTime = 0;

void measureVolume(){
  double vol;
  if (xQueueReceive(slm_queue, &vol, 0) == pdPASS) {
    if (!isnan(vol)) {
      if        (vol < 50) {
        if (iLeds2 != BLUE_ON) {
          iLeds2 = BLUE_ON;
          leds2[iLeds2].Reset();
        }
      } else if (vol < 70) {
        if (iLeds2 != GREEN_ON) {
          iLeds2 = GREEN_ON;
          leds2[iLeds2].Reset();
        }
      } else if (vol < 90) {
        if (iLeds2 != YELLOW_ON) {
          iLeds2 = YELLOW_ON;
          leds2[iLeds2].Reset();
        }
      } else {
        iLeds2 = RED_ON;
        leds2[iLeds2].Reset();
      }      

      if ((millis() - lastVolumeTime) > VolumeDelay) {
        lastVolumeTime = millis();
        publishMessage("vol", String(vol), true);
      }
    }
  } 
}


void measureCO2() {
  int co2;
  if (xQueueReceive(mhz19_queue, &co2, 0) == pdPASS) {
    if        (co2 < 700) {
      if (iLeds1 != BLUE_ON) {
        iLeds1 = BLUE_ON;
        leds1[iLeds1].Reset();    
      }
    } else if (co2 < 1000) {
      if (iLeds1 != GREEN_ON) {
        iLeds1 = GREEN_ON;
        leds1[iLeds1].Reset();    
      }
    } else if (co2 < 1500) {
      if (iLeds1 != YELLOW_ON) {
        iLeds1 = YELLOW_ON;
        leds1[iLeds1].Reset();    
      }
    } else {
      iLeds1 = RED_ON;
      leds1[iLeds1].Reset();
    }
  
    if (co2 != 0 && co2 != 5000) {
      publishMessage("co2", String(co2), true);
    }      
  }
}


void measureBME() {
  bme_msg_t bme_msg;
  if (xQueueReceive(bme_queue, &bme_msg, 0) == pdPASS) {
    publishMessage("temperature",   String(bme_msg.temperature / 100.0 ), true);
    publishMessage("humidity",      String(bme_msg.humidity    / 1000.0), true);
    publishMessage("pressure",      String(bme_msg.pressure    / 100.0 ), true);
    publishMessage("gasResistance", String(bme_msg.gas                 ), true);
  }
}


void playToneOnButton(){
  btn_msg_t btn_msg;
  tone_msg_t tone_msg;

  if (xQueueReceive(button_queue, &btn_msg, 0) == pdPASS) {
    if(btn_msg.value == HIGH) {
      tone_msg.frequency = NOTE_C4;
      tone_msg.duration = 16;    
    } else {
      tone_msg.frequency = NOTE_C3;
      tone_msg.duration = 16;     
    }
    xQueueSend( buzzer_queue, &tone_msg, 0 );  
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  slm_queue    = xQueueCreate(I2S_XQUEUE_SIZE    , sizeof(double));
  mhz19_queue  = xQueueCreate(MHZ19_XQUEUE_SIZE  , sizeof(int));
  bme_queue    = xQueueCreate(BME_XQUEUE_SIZE    , sizeof(bme_msg_t));
  buzzer_queue = xQueueCreate(BUZZER_XQUEUE_SIZE , sizeof(tone_msg_t));
  button_queue = xQueueCreate(BUTTON_XQUEUE_SIZE , sizeof(btn_msg_t));

  xTaskCreatePinnedToCore(led_task,            "LED",    LED_TASK_STACK    , NULL, LED_TASK_PRI,    &task_led,    1);
  xTaskCreatePinnedToCore(buzzer_task,         "BUZZER", BUZZER_TASK_STACK , NULL, BUZZER_TASK_PRI, &task_buzzer, 1);
  xTaskCreatePinnedToCore(button_task,         "BUTTON", BUTTON_TASK_STACK , NULL, BUTTON_TASK_PRI, &task_button, 0);
  xTaskCreatePinnedToCore(mic_i2s_reader_task, "SLM",    I2S_TASK_STACK    , NULL, I2S_TASK_PRI,    &task_slm,    0);
  xTaskCreatePinnedToCore(mhz19_reader_task,   "MHZ-19", MHZ19_TASK_STACK  , NULL, MHZ19_TASK_PRI,  &task_mhz19,  0);
  xTaskCreatePinnedToCore(bme_task,            "BME",    BME_TASK_STACK    , NULL, BME_TASK_PRI,    &task_bme,    0);
  xTaskCreatePinnedToCore(wifi_task,           "WIFI",   WIFI_TASK_STACK   , NULL, WIFI_TASK_PRI,   &task_wifi,   1);
  xTaskCreatePinnedToCore(mqtt_task,           "MQTT",   MQTT_TASK_STACK   , NULL, MQTT_TASK_PRI,   &task_mqtt,   1);

  randomSeed(micros());
}


void loop() {
  playToneOnButton();
  measureVolume();
  measureCO2();
  measureBME();  
  delay(1);
}
