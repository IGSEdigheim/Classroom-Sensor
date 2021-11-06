#define BUZZER_PIN 26
#define TONE_CHANNEL 0 // 15

#include "pitches.h"

unsigned long buzzer_interval = 10;

// FreeRTOS Queue
QueueHandle_t buzzer_queue = NULL;
#define BUZZER_XQUEUE_SIZE 4

// FreeRTOS Task
static TaskHandle_t task_buzzer;
#define BUZZER_TASK_PRI   4
#define BUZZER_TASK_STACK 1024

struct tone_msg_t {
  unsigned int frequency;
  unsigned long duration;
};

void noTone(uint8_t pin)
{
    ledcDetachPin(pin);
    ledcWrite(TONE_CHANNEL, 0);
}


void tone(uint8_t pin, unsigned int frequency, unsigned long duration)
{
    if (ledcRead(TONE_CHANNEL)) {
        log_e("Tone channel %d is already in use", ledcRead(TONE_CHANNEL));
        return;
    }
    ledcAttachPin(pin, TONE_CHANNEL);
    ledcWriteTone(TONE_CHANNEL, frequency);
    if (duration) {
        delay(duration);
        noTone(pin);
    }    
}



void buzzer_task(void* parameter) {
  String taskMessage = "+ Buzzer-Task running on core ";
  taskMessage = taskMessage + xPortGetCoreID();
  Serial.println(taskMessage);
  
  pinMode(BUZZER_PIN, OUTPUT);
  
  for( ;; ) {
    //Serial.println(": Buzzer Remaining Stack" + String(uxTaskGetStackHighWaterMark( NULL )));

    tone_msg_t tone_msg;
    if (xQueueReceive(buzzer_queue, &tone_msg, 0) == pdPASS) {
      int noteDuration = 1000 / tone_msg.duration;
      tone(BUZZER_PIN, tone_msg.frequency, noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(BUZZER_PIN);
    }
    delay( buzzer_interval );
  }  
}
