#include "Zanshin_BME680.h"   ///< The BME680 sensor library

unsigned long bme_interval = 15000;
const uint8_t  POWER_PIN   = A13;

// FreeRTOS Queue
QueueHandle_t bme_queue = NULL;
#define BME_XQUEUE_SIZE 1

// FreeRTOS Task
static TaskHandle_t task_bme;
#define BME_TASK_PRI   4
#define BME_TASK_STACK (1024 * 2) // ok

struct bme_msg_t {
  int32_t temperature;              ///< BME680 temperature value
  int32_t humidity;                 ///< BME680 humidity value
  int32_t pressure;                 ///< BME680 pressure value
  int32_t gas;                      ///< BME680 gas resistance value
};

BME680_Class BME680;                   ///< Create an instance of the BME680 class



void bme_init (void) {
  while (!BME680.begin(I2C_STANDARD_MODE)) {  // Start using I2C, use first device found
    //Serial.print("- Unable to find BME680.\n");
    delay(1000);
  }  // of loop until device is located

  //Serial.print("- Setting 16x oversampling for all sensors\n");
  BME680.setOversampling(TemperatureSensor, Oversample16);
  BME680.setOversampling(HumiditySensor, Oversample16);
  BME680.setOversampling(PressureSensor, Oversample16);
  //Serial.print("- Setting IIR filter to a value of 4 samples\n");
  BME680.setIIRFilter(IIR4);
  //Serial.print(F("- Setting gas measurement to 320\xC2\xB0\x43 for 150ms\n"));
  BME680.setGas(320, 150);
}

void bme_task(void* parameter) {
  String taskMessage = "+ BME-Task running on core ";
  taskMessage = taskMessage + xPortGetCoreID();
  Serial.println(taskMessage);
  
  bme_init();
  
  for( ;; ) {
    //Serial.println(": BME Remaining Stack " + String(uxTaskGetStackHighWaterMark( NULL )));
    bme_msg_t bme_msg;
    BME680.getSensorData(bme_msg.temperature, bme_msg.humidity, bme_msg.pressure, bme_msg.gas);  // Get readings
    xQueueSend( bme_queue, &bme_msg, 0 );
    delay( bme_interval );
  }
}

    /*
    String jsonData;
    jsonData = "{ \"temperature\": "   + String(bme_msg.temperature / 100.0)  +
               ", \"humidity\": "      + String(bme_msg.humidity    / 1000.0) +
               ", \"pressure\": "      + String(bme_msg.pressure    / 100.0)  + 
             //", \"Altitude\": "      + String(altitude)                     +
               ", \"gasResistance\": " + String(bme_msg.gas                )  + " }";
    */
