#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ConfigManager.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"
#include "common.h"

//measurement duration is approx Buffer_SZ/25 (in seconds). And Max buffer size is 100 (4 seconds)
#define MAX30102_MEASUREMENT_BUFFER_SZ   100

bool deleteOldCreds = 0;

typedef struct
{ 
  char ssid[32];
  char pwd[64];
  char hospitalName[64]; 
  char buildingNum[32]; 
  char floorNum[32]; 
  char roomNum[32]; 
  char bedNum[32];
}stc_device_congfig;

stc_device_congfig g_devCfg;

void setup()
{
  delay(5000);
  lcdSetup();
  Serial.begin(115200); // initialize serial communication at 115200 bits per second:
  
  if(deleteOldCreds)
  {
    EEPROM.begin(512);// cache the first 512 bytes of FLASH into RAM
    for(int i = 0;i<512;i++)
      EEPROM.write(i,0);
    EEPROM.end();
    ESP.reset();
  }
  
  EEPROM.begin(512);// cache the first 512 bytes of FLASH into RAM
  EEPROM.get(0,g_devCfg); // reload the previous data
  wifi_setup();

  delay(500);
  max30102_Init();
  Serial.println(F("Attach sensor to finger with rubber band."));
  max30102_Setup();
  delay(500);
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
