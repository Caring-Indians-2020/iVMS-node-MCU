MAX30105 particleSensor;

//#define ENABLE_RAW_IR_RED_DATA_OUT // uncomment to get continuous raw data on Serial monitor
#define MAX_BUFFER_SZ 110
#define MAX_DETECTABLE_HEART_RATE 250  // Will detect heart Rates below 200 only
// minimum milliseconds between two switches of PPG signal
/* Thought process is as follows:
 *  This system can detect a max heart rate of 250 bpm. This translates to 4.167 bps.
 *  Thus we are expecting 1 beat to take a minimum of 240 ms. This translates to a minimum gap 
 *  of 120 ms between two "switches".
 */
#define MIN_SWITCH_TIME 120
#define MIN_RESET_INTERVAL 5000 //in milli seconds
long max30102_timeSinceLastReset;
uint32_t  irBuffer[MAX_BUFFER_SZ]; //infrared LED sensor data
uint32_t  redBuffer[MAX_BUFFER_SZ];  //red LED sensor data
uint32_t  timeBuf[MAX_BUFFER_SZ] ;
int32_t ppg[MAX_BUFFER_SZ];
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

    //TODO: Handle this better
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

void max30102_printPPG_to_UART(void)
{
  for(byte i =0; i<MAX30102_MEASUREMENT_BUFFER_SZ;i++)
  {
    Serial.print("time = ");
    Serial.print(timeBuf[i], DEC);
    Serial.print(F(", PPG = "));
    Serial.println(ppg[i], DEC);  
  }
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
  uint32_t movingAvgIr = 0, crossPoint = 0, lastSwitchTime = (timeBuf[1]- 2*MIN_SWITCH_TIME);
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
      if((timeBuf[i] - lastSwitchTime) > MIN_SWITCH_TIME)
      {
        if((irBuffer[i] > movingAvgIr)&&(signalPos == 0))
        {
          numSwitches++;
          lastSwitchTime = timeBuf[i];
          crossPoint = movingAvgIr;
          signalPos = 1;          
        }
        else if ((irBuffer[i] < movingAvgIr)&&(signalPos == 1))
        {
          numSwitches++;
          lastSwitchTime = timeBuf[i];
          crossPoint = movingAvgIr;
          signalPos = 0;
        }
      }
    }

    //Level PPG about the zero 
    if(crossPoint>0)
    {
      ppg[i] = irBuffer[i] - crossPoint;
    }
    else
    {
      ppg[i] = 0;
    }

  }

  heartRate = ((float)numSwitches/2)*(60000/(float)daqTime);
}

void max30102_reset(void)
{
  //Reset FW and hardware
  daqTime = 0;
  spo2 = 0; //SPO2 value
  validSPO2 = 0; //indicator to show if the SPO2 calculation is valid
  heartRate = 0; //heart rate value
  validHeartRate = 0; //indicator to show if the heart rate calculation is valid
  //Need to check how to reset the hardware
}

void max30102_measure(int32_t bufLen)
{
  long ir, red, timeVal;
  byte numRetry = 0;

  max30102_reset();
  max30102_readSensor(&ir,&red,&timeVal);
  
  while((ir<50000)&&(numRetry <10))
  {
    //Try 10 times to check if finger is present.
    max30102_readSensor(&ir,&red,&timeVal);
    numRetry++;
  };

  if(ir>50000)
  {
    max30102_AcquireSamples(bufLen);
    //Use library API to get spO2 concentration
    maxim_heart_rate_and_oxygen_saturation((uint32_t *)irBuffer, bufLen, (uint32_t *)redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    max30102_processDataForHeartRate(bufLen);
  }
}
