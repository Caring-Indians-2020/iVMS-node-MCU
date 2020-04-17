#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"

#define ENABLE_RAW_IR_RED_DATA_OUT // uncomment to get continuous raw data on Serial monitor

//measurement duration is approx Buffer_SZ/25 (in seconds). And Max buffer size is 200 ( 8 seconds)
#define MAX30102_MEASUREMENT_BUFFER_SZ  50 

void setup()
{
  lcdSetup();
  Serial.begin(115200); // initialize serial communication at 115200 bits per second:
  delay(5000);
  max30102_Init();
  Serial.println(F("Attach sensor to finger with rubber band."));
  max30102_Setup();
  delay(5000);
}

void loop()
{
  max30102_measure(MAX30102_MEASUREMENT_BUFFER_SZ);

  if(0)//(max30102_isDataValid())
  {
      Serial.print(F("SPO2="));
      Serial.print(max30102_getSpO2(), DEC);
      Serial.print(F(", HR="));
      Serial.print(max30102_getHeartRate(), DEC);
      Serial.println();
  }
  
  refreshLcd();
}
