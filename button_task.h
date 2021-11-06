#define BTN1 35
#define BTN2 34

unsigned long button_interval = 10;
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

// FreeRTOS Queue
QueueHandle_t button_queue = NULL;
#define BUTTON_XQUEUE_SIZE 4

// FreeRTOS Task
static TaskHandle_t task_button;
#define BUTTON_TASK_PRI   4
#define BUTTON_TASK_STACK 1024

struct btn_msg_t {
  int button;
  int value;
};

int button1State;
int button2State;
int lastButton1State = HIGH;
int lastButton2State = HIGH;
unsigned long lastDebounceTimeBtn1 = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTimeBtn2 = 0;  // the last time the output pin was toggled


void button_init(void) {
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
}

void button_task(void* parameter) {
  String taskMessage = "+ Button-Task running on core ";
  taskMessage = taskMessage + xPortGetCoreID();
  Serial.println(taskMessage);
  
  button_init();
  for( ;; ) {
    //Serial.println(": Button Remaining Stack" + String(uxTaskGetStackHighWaterMark( NULL )));

    btn_msg_t reading1;
    reading1.button = 1;
    btn_msg_t reading2;
    reading2.button = 2;
    
    reading1.value = digitalRead(BTN1);
    reading2.value = digitalRead(BTN2);

    if (reading1.value != lastButton1State)
        lastDebounceTimeBtn1 = millis();
    if (reading2.value != lastButton2State)
        lastDebounceTimeBtn2 = millis();
    
    if ((millis() - lastDebounceTimeBtn1) > debounceDelay) {
  
      // if the button state has changed:
      if (reading1.value != button1State) {
        button1State = reading1.value;
  
        if (reading1.value == LOW) {
          Serial.println(": Button 1 pressed");
        } else {
          Serial.println(": Button 1 released");
        }
        
        xQueueSend( button_queue, &reading1, 0 );
      }
    }

    if ((millis() - lastDebounceTimeBtn2) > debounceDelay) {
  
      // if the button state has changed:
      if (reading2.value != button2State) {
        button2State = reading2.value;
  
        if (reading2.value == LOW) {
          Serial.println(": Button 2 pressed");
        } else {
          Serial.println(": Button 2 released");
        }
        
        xQueueSend( button_queue, &reading2, 0 );
      }
    }

    lastButton1State = reading1.value;
    lastButton2State = reading2.value;
    delay( button_interval );
  }  
}
