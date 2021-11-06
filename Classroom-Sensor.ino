#define LED1B 12
#define LED1G 14
#define LED1R 27
#define LED2B 25
#define LED2G 33
#define LED2R 32

#include "cred.h"
#include "wifi_task.h"
#include "mqtt_task.h"
#include "slm_task.h"
#include "mhz19_task.h"
#include "button_task.h"
#include "buzzer_task.h"

unsigned long VolumeDelay = 5000;
unsigned long lastVolumeTime = 0;

void measureVolume(){
  //Serial.println("measureVolume()");
  double vol;
  if (xQueueReceive(slm_queue, &vol, 0) == pdPASS) {
    if (!isnan(vol)) {
      if        (vol < 50) {
        /*
        if (iLeds2 != BLUE_ON) {
          iLeds2 = BLUE_ON;
          leds2[iLeds2].Reset();
        }
        */
        digitalWrite(LED2B, HIGH);
        digitalWrite(LED2G, LOW);
        digitalWrite(LED2R, LOW);
      } else if (vol < 60) {
        /*
        if (iLeds2 != GREEN_ON) {
          iLeds2 = GREEN_ON;
          leds2[iLeds2].Reset();
        }
        */
        digitalWrite(LED2B, LOW);
        digitalWrite(LED2G, HIGH);
        digitalWrite(LED2R, LOW);
        
      } else if (vol < 70) {
        /*
        if (iLeds2 != YELLOW_ON) {
          iLeds2 = YELLOW_ON;
          leds2[iLeds2].Reset();
        }
        */
        digitalWrite(LED2B, LOW);
        digitalWrite(LED2G, HIGH);
        digitalWrite(LED2R, HIGH);
        
      } else {
        /*
        iLeds2 = RED_ON;
        leds2[iLeds2].Reset();
        */
        digitalWrite(LED2B, LOW);
        digitalWrite(LED2G, LOW);
        digitalWrite(LED2R, HIGH);
        
      }      

      if ((millis() - lastVolumeTime) > VolumeDelay) {
        lastVolumeTime = millis();

        mqtt_msg_t pub_msg;
        pub_msg.topic = "vol";
        pub_msg.message = String(vol);
        pub_msg.retain = true;
        xQueueSend( mqtt_queue, &pub_msg, 0 );
      }
    }
  } 
}


void measureCO2() {
  //Serial.println("measureCO2()");
  int co2;
  if (xQueueReceive(mhz19_queue, &co2, 0) == pdPASS) {
    if        (co2 < 700) {
      /*
      if (iLeds1 != BLUE_ON) {
        iLeds1 = BLUE_ON;
        leds1[iLeds1].Reset();    
      }
      */
      digitalWrite(LED1B, HIGH);
      digitalWrite(LED1G, LOW);
      digitalWrite(LED1R, LOW);
    } else if (co2 < 1000) {
      /*
      if (iLeds1 != GREEN_ON) {
        iLeds1 = GREEN_ON;
        leds1[iLeds1].Reset();    
      }*/
      digitalWrite(LED1B, LOW);
      digitalWrite(LED1G, HIGH);
      digitalWrite(LED1R, LOW);
    } else if (co2 < 1500) {
      /*
      if (iLeds1 != YELLOW_ON) {
        iLeds1 = YELLOW_ON;
        leds1[iLeds1].Reset();    
      }
      */
      digitalWrite(LED1B, LOW);
      digitalWrite(LED1G, HIGH);
      digitalWrite(LED1R, HIGH);
    } else {
      /*
      iLeds1 = RED_ON;
      leds1[iLeds1].Reset();
      */
      digitalWrite(LED1B, LOW);
      digitalWrite(LED1G, LOW);
      digitalWrite(LED1R, HIGH);
    }
  
    if (co2 != 0 && co2 != 5000) {
      mqtt_msg_t pub_msg;
      pub_msg.topic = "co2";
      pub_msg.message = String(co2);
      pub_msg.retain = true;
      xQueueSend( mqtt_queue, &pub_msg, 0 );
    }      
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

  pinMode(LED1B, OUTPUT);
  pinMode(LED1G, OUTPUT);
  pinMode(LED1R, OUTPUT);
  pinMode(LED2B, OUTPUT);
  pinMode(LED2G, OUTPUT);
  pinMode(LED2R, OUTPUT);

  slm_queue    = xQueueCreate(I2S_XQUEUE_SIZE    , sizeof(double));
  mhz19_queue  = xQueueCreate(MHZ19_XQUEUE_SIZE  , sizeof(int));
  mqtt_queue   = xQueueCreate(MQTT_XQUEUE_SIZE   , sizeof(mqtt_msg_t));
  button_queue = xQueueCreate(BUTTON_XQUEUE_SIZE , sizeof(btn_msg_t));
  buzzer_queue = xQueueCreate(BUZZER_XQUEUE_SIZE , sizeof(tone_msg_t));

  xTaskCreate            (buzzer_task,         "BUZZER", BUZZER_TASK_STACK, NULL, BUZZER_TASK_PRI, &task_buzzer);
  xTaskCreatePinnedToCore(mic_i2s_reader_task, "SLM",    I2S_TASK_STACK,    NULL, I2S_TASK_PRI,    &task_slm,    0);
  xTaskCreatePinnedToCore(mhz19_reader_task,   "MHZ-19", MHZ19_TASK_STACK,  NULL, MHZ19_TASK_PRI,  &task_mhz19,  0);
  xTaskCreatePinnedToCore(wifi_task,           "WIFI",   WIFI_TASK_STACK,   NULL, WIFI_TASK_PRI,   &task_wifi,   1);
  xTaskCreatePinnedToCore(mqtt_task,           "MQTT",   MQTT_TASK_STACK,   NULL, MQTT_TASK_PRI,   &task_mqtt,   1);
  xTaskCreate            (button_task,         "BUTTON", BUTTON_TASK_STACK, NULL, BUTTON_TASK_PRI, &task_button);

  randomSeed(micros());
}


void loop() {
  //leds1[iLeds1].Update();
  //leds2[iLeds2].Update();

  playToneOnButton();
  measureVolume();
  measureCO2();
  
  delay(1);
}
