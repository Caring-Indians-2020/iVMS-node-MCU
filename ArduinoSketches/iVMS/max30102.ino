MAX30105 particleSensor;

#define MAX_BUFFER_SZ 110

uint32_t  irBuffer[MAX_BUFFER_SZ]; //infrared LED sensor data
uint32_t  redBuffer[MAX_BUFFER_SZ];  //red LED sensor data
uint32_t  timeBuf[MAX_BUFFER_SZ] ;

long  daqTime = 0;  //Total Data Acquisition time

int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid


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
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 100kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }
}
void max30102_Setup(void)
{
  byte ledBrightness = 0x1F; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32. Number of samples averaged together on the chip
  byte ledMode = 3; //Options: 2 = Hear Rate Only, 3 = SpO2 mode, 7 = Multi-LED mode
  byte sampleRate = 400; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200. Number of RED & IR pulses sampled per second. With smaller sample rate, We get higher resolution.
  int pulseWidth = 411; //Options: 69, 118, 215, 411. Decides the Sample Rate Upper Bound. (Max_sampleRate = kludeFactor* 1/(2*pulseWidth))
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384. Sets the LED drive current per bit

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void max30102_printRawDataToUart(uint32_t red, uint32_t ir, uint32_t timeStamp)
{
  if(ir > 50000)
  {
      //send samples and calculation result to terminal program through UART
      Serial.print("RED LED = ");
      Serial.print(red, DEC);
      Serial.print(F(", IR LED = "));
      Serial.print(ir, DEC);
      Serial.print(F(", timestamp = "));
      Serial.println(timeStamp, DEC);
      
  }
}

/*
 * Sensor reading API. Make sure to not add any delays, debug info etc here.
 * This has to be quick!!
 */
void max30102_readSensor(long* ir, long* red, long* timestamp)
{
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //Check the sensor for new data
    
    *red = particleSensor.getRed();
    *ir = particleSensor.getIR();
    *timestamp =  millis() - daqTime;
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
}

/*
 * Data Acquisition API. Make sure to not add any delays, debug info etc here.
 * This has to be successive!!
 */
void max30102_AcquireSamples(unsigned int detBufferLength)
{
  daqTime = millis();
  for (byte i = 0 ; i < detBufferLength ; i++)
  {
    max30102_readSensor((long*)&irBuffer[i],(long *)&redBuffer[i],(long *)&timeBuf[i]);
  }
  daqTime = millis() - daqTime;
}

void max30102_processDataForHeartRate(unsigned int bufLen)
{
  uint32_t movingAvgIr = 0;
  byte signalPos =0, numSwitches =0;

  for(byte i = 0; i < bufLen;i++)
  {
    #ifdef ENABLE_RAW_IR_RED_DATA_OUT
    max30102_printRawDataToUart(redBuffer[i],irBuffer[i],timeBuf[i]);
    #endif
    movingAvgIr +=irBuffer[i];
    if(i>0)
    {
      movingAvgIr /=2;
      if((irBuffer[i] > movingAvgIr)&&(signalPos == 0))
      {
        numSwitches++;
        signalPos = 1;
      }
      else if ((irBuffer[i] < movingAvgIr)&&(signalPos == 1))
      {
        numSwitches++;
        signalPos = 0;
      }
    }
  }
  heartRate = ((float)numSwitches/2)*(60000/(float)daqTime);
}

void max30102_measure(int32_t bufLen)
{
  max30102_AcquireSamples(bufLen);
  //Use library API to get spO2 concentration
  maxim_heart_rate_and_oxygen_saturation((uint32_t *)irBuffer, bufLen, (uint32_t *)redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  max30102_processDataForHeartRate(bufLen);
}
