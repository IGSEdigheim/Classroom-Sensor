// NOT IN USE RIGHT NOW !


// BME680
#include "bsec.h"

// Helper functions declarations
void checkIaqSensorStatus(void);
void errLeds(void);

// Create an object of the class Bsec
Bsec iaqSensor;
unsigned long bme680_measure_interval = 5000;

// FreeRTOS Queue
QueueHandle_t bme680_queue = NULL;
#define BME680_XQUEUE_SIZE 1

// FreeRTOS Task
static TaskHandle_t task_bme680;
#define BME680_TASK_PRI   8
#define BME680_TASK_STACK 4096

struct bme_sens_t {
  float temperature;
  float humidity;
  float pressure;
  float gasResistance;
  float iaq;
  uint8_t iaqAccuracy;
  float staticIaq;
  uint8_t staticIaqAccuracy;
  float co2Equivalent;
  uint8_t co2Accuracy;
  float breathVocEquivalent;
  uint8_t breathVocAccuracy;
  float gasPercentage;
  uint8_t gasPercentageAcccuracy;
};


void bme_i2c_init(void)
{
  Wire.begin();

  iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);
  String output = ": BSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial.println(output);
  checkIaqSensorStatus();

  bsec_virtual_sensor_t sensorList[10] = {
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_COMPENSATED_GAS,
    BSEC_OUTPUT_GAS_PERCENTAGE,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };

  iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();
}

// Helper function definitions
void checkIaqSensorStatus(void)
{
  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK) {
      String output = "BSEC error code : " + String(iaqSensor.status);
      Serial.println(output);
      //for (;;)
      //  errLeds(); /* Halt in case of failure */
    } else {
      String output = "BSEC warning code : " + String(iaqSensor.status);
      Serial.println(output);
    }
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      String output = "BME680 error code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
      //for (;;)
      //  errLeds(); /* Halt in case of failure */
    } else {
      String output = "BME680 warning code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
    }
  }
}

void errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}


bme_sens_t bme680_measure() {
  if (iaqSensor.run()) { // If new data is available
    
    bme_sens_t b;

    b.temperature             = iaqSensor.temperature;
    b.humidity                = iaqSensor.humidity;
    b.pressure                = iaqSensor.pressure;
    b.gasResistance           = iaqSensor.gasResistance;
    b.iaq                     = iaqSensor.iaq;
    b.iaqAccuracy             = iaqSensor.iaqAccuracy;
    b.staticIaq               = iaqSensor.staticIaq;
    b.staticIaqAccuracy       = iaqSensor.staticIaqAccuracy;
    b.co2Equivalent           = iaqSensor.co2Equivalent;
    b.co2Accuracy             = iaqSensor.co2Accuracy;
    b.breathVocEquivalent     = iaqSensor.breathVocEquivalent;
    b.breathVocAccuracy       = iaqSensor.breathVocAccuracy;
    b.gasPercentage           = iaqSensor.gasPercentage;
    b.gasPercentageAcccuracy  = iaqSensor.gasPercentageAcccuracy;
    
    return b;
    
  } else {
    checkIaqSensorStatus();
  }
}


void bme680_reader_task(void* parameter) {
  String taskMessage = "+ BME680-Task running on core ";
  taskMessage = taskMessage + xPortGetCoreID();
  Serial.println(taskMessage);

  
  bme_i2c_init();  
   
  for( ;; ) {
    //Serial.println(": BME680 Remaining Stack" + String(uxTaskGetStackHighWaterMark( NULL )));
    bme_sens_t bme_result = bme680_measure();
    xQueueSend(bme680_queue, &bme_result, 0);
    delay( bme680_measure_interval );
  } 
}


// bme680_queue = xQueueCreate(BME680_XQUEUE_SIZE , sizeof(bme_sens_t));
// xTaskCreate(bme680_reader_task,  "BME680", BME680_TASK_STACK, NULL, BME680_TASK_PRI, &task_bme680);

/*
void measureBME680() {
  //Serial.println("measureBME680()");
  bme_sens_t sensor_val;
  if (xQueueReceive(bme680_queue, &sensor_val, 0) == pdPASS)
  {
    String output = "";
    
    output = "{";
    output +=  "\"temperature\": " + String(sensor_val.temperature);// + " °C";
    output += ",\"humidity\": " + String(sensor_val.humidity);// + " %";
    output += ",\"pressure\": " + String(sensor_val.pressure / 100.0);// + " hPa";
    output += ",\"gasResistance\": " + String(sensor_val.gasResistance );// + " Ω";

    output += ",\"iaq\": {";
    output +=  "\"value\": " + String(sensor_val.iaq);
    output += ",\"accuracy\": " + String(sensor_val.iaqAccuracy); // 3==BSEC calibrated successfully
    output += "}";
    output += ",\"staticIaq\": {";
    output +=  "\"value\": " + String(sensor_val.staticIaq);
    output += ",\"accuracy\": " + String(sensor_val.staticIaqAccuracy);
    output += "}";
    output += ",\"co2Equivalent\": {";
    output +=  "\"value\": " + String(sensor_val.co2Equivalent);// + " ppm";
    output += ",\"accuracy\": " + String(sensor_val.co2Accuracy);
    output += "}";
    output += ",\"breathVocEquivalent\": {";
    output +=  "\"value\": " + String(sensor_val.breathVocEquivalent);// + " ppm";
    output += ",\"accuracy\": " + String(sensor_val.breathVocAccuracy);
    output += "}";
    output += ",\"gasPercentage\": {";
    output +=  "\"value\": " + String(sensor_val.gasPercentage);// + " %";
    output += ",\"accuracy\": " + String(sensor_val.gasPercentageAcccuracy);
    output += "}";
    output += "}";

    //publishMessage("bme680",output, true);

    mqtt_msg_t pub_msg;
    pub_msg.topic = "bme680";
    pub_msg.message = output;
    pub_msg.retain = true;
    xQueueSend( mqtt_queue, &pub_msg, 0 );
  }
}
*/
