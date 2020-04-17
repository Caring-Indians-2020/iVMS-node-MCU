MAX30105 particleSensor;

#define MAX_BUFFER_SZ 200

uint32_t irBuffer[MAX_BUFFER_SZ]; //infrared LED sensor data
uint32_t redBuffer[MAX_BUFFER_SZ];  //red LED sensor data
byte rates[MAX_BUFFER_SZ]; //Array of heart rates

int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid

byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
unsigned long beatAvg;
int32_t heartRate1; //heart rate value from second Algo

int32_t max30102_getSpO2(void)
{
  return spo2;
}

int32_t max30102_getHeartRate(void)
{
  return heartRate;
}

bool max30102_isDataValid(void)
{
  return validSPO2;
}

void max30102_Init(void)
{
    // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) //Use default I2C port, 100kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }
}
void max30102_Setup(void)
{
  byte ledBrightness = 0x1F; //Options: 0=Off to 255=50mA
  byte sampleAverage = 2; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 3; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 400; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void max30102_printRawDataToUart(uint32_t red, uint32_t ir)
{
      //send samples and calculation result to terminal program through UART
      //Serial.print(F("red="));
      //Serial.print(red, DEC);
      //Serial.print(F(", ir= "));
      Serial.print(ir, DEC);
      Serial.println();
}

void max30102_readSensor(long* ir, long* red)
{
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //Check the sensor for new data
    
    *red = particleSensor.getRed();
    *ir = particleSensor.getIR();

    particleSensor.nextSample(); //We're finished with this sample so move to next sample
}

void max30102_AcquireSamples(unsigned int detBufferLength)
{
  //read the first 400 samples, and determine the signal range
  rateSpot = 0;
  for (byte i = 0 ; i < detBufferLength ; i++)
  {
    max30102_readSensor((long*)&irBuffer[i],(long *)&redBuffer[i]);
    
  #ifdef ENABLE_RAW_IR_RED_DATA_OUT
    max30102_printRawDataToUart(redBuffer[i],irBuffer[i]);
  #endif
  }
}

void max30102_measure(int32_t bufLen)
{
  max30102_AcquireSamples(bufLen);
  //Use library API to get spO2 concentration
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufLen, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

}
