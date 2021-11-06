#include "MHZ19.h"            //https://github.com/WifWaf/MH-Z19
MHZ19 myMHZ19;                // Constructor for library
HardwareSerial mySerial(2);   // (ESP32 Example) create device to MH-Z19 serial
#define BAUDRATE 9600         // Device to MH-Z19 Serial baudrate (should not be changed)

unsigned long mhz19_measure_interval = 5000;

// FreeRTOS Queue
QueueHandle_t mhz19_queue = NULL;
#define MHZ19_XQUEUE_SIZE 1

// FreeRTOS Task
static TaskHandle_t task_mhz19;
#define MHZ19_TASK_PRI   4
#define MHZ19_TASK_STACK 1536 // 1024 is almost to less



void mhz19_serial_init(void) {
  mySerial.begin(BAUDRATE);
  myMHZ19.begin(mySerial);   // *Serial(Stream) refence must be passed to library begin().
  myMHZ19.autoCalibration(); // Turn auto calibration ON (OFF autoCalibration(false))
}
  

void mhz19_reader_task(void* parameter) {
  String taskMessage = "+ MHZ19-Task running on core ";
  taskMessage = taskMessage + xPortGetCoreID();
  Serial.println(taskMessage);
  
  mhz19_serial_init();
  for( ;; ) {
    //Serial.println(": MHZ-19 Remaining Stack" + String(uxTaskGetStackHighWaterMark( NULL )));
    int co2 = myMHZ19.getCO2();
    xQueueSend( mhz19_queue, &co2, 0 );
    delay( mhz19_measure_interval );
  }  
}
