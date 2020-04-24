#include <PubSubClient.h>

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"

//measurement duration is approx Buffer_SZ/25 (in seconds). And Max buffer size is 100 (4 seconds)
#define MAX30102_MEASUREMENT_BUFFER_SZ   100

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

  if(max30102_isDataValid())
  {
      refreshLcd();
      max30102_printPPG_to_UART();
  }
  else
  {
    lcd_fingerMissing();
  }
}
