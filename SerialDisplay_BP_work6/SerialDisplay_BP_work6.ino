/**
 * CK101 BP sensor based measurement - automatic trigger of measurement on a programmed timer and readout + display on an I2C lcd 16x2 display
 * Displays text sent over the serial port (e.g. from the Serial Monitor)
 * 
 */
 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte ind0 = 0;  // initial ck101 BP sensor initialized message index for char array
byte ind1 = 0;  // control text going to write lcd function begins with 'B'
unsigned long x, y;
x = 120000;     // trigger measurement every x millisec
y = 62000;      // trigger readout of measurement from breakout board ONCE, y millisec after x event is triggered 
char ch, ch2, txtbuffer[32], txttolcd[32];
unsigned long previousMillis = 0;        // timer value for automatic startck measurement every x milliseconds
unsigned long ck101t_read;               // timer value for reading measured values after y milliseconds period alloted for measurement
bool isB = false;
bool readCKmeas = false;
void setup()
{
	lcd.begin();
	lcd.backlight();
  // Initialize the serial port at a speed of 9600 baud
	Serial.begin(9600); 
  Serial.swap();      // shift serial communication of UART0 port to GPIO13/D7_RX and GPIO15/D8_Tx
}

void writetolcd(char b[], int lt) //inputs are char array and # of char to be written
{
  byte ind2 = 0;  // index for writing data to lcd 16 char per line
  lcd.clear();
  lcd.print(lt);  // display # of char received via serial and write to position 0,0 and 1,0 
  lcd.setCursor(3,0); //start writing data from position  position 3,0
  while (ind2 < 13){
    if (b[ind2] != 0x20){  // ignore spaces in received data, they take up too many chars
    lcd.write(b[ind2]);
    }
    ind2++;
  }
  lcd.setCursor(0,1);       // set cursor to line 2
  while (ind2 < lt-1){      // ignore extra space char received at end of data
    if (b[ind2] != 0x20){   // ignore spaces in received data, they take up too many chars
    lcd.write(b[ind2]);
    }
    ind2++;
  }
  
}

bool startck()        // trigger measurement of BP by sending $ char to breakout board
{
  Serial.write(0x24); // $ sign sent to CK101 board to start measurement
  return(true);       // flag to send ~ char for reading measurement data only ONCE per measurement
}

void loop()
{
    unsigned long currentMillis = millis();
    delay(100);
    if (Serial.available()) {     // Write all characters received with the serial port to the LCD.
      ind0 = 0;                   // counter for char array of received serial data
      isB = false;
      delay(100);
      while (Serial.available() > 0) {
        //lcd.write(Serial.read());
        ch = Serial.read();
        txtbuffer[ind0] = ch;
        ind0++;

      }
      
      //for (byte x = 0; x < ind0; x++){
      //  ch2 = txtbuffer[x];
      //  if (ch2 == 0x42){
      //    isB = true;
      //  }
      // if (isB){
      //    txttolcd[y] = ch2;
      //    y++;  
      //  }
    }
    if (ind0 > 10){
    writetolcd(txtbuffer, ind0);    // only write to lcd if received data is > 10 char, sometimes smaller sized data is also sent.
                                    // measurement data is always 29 char
    }
   
    if (currentMillis - previousMillis > x){         // 120 seconds check to startck measurement
          previousMillis = currentMillis;
          ck101t_read = currentMillis;
          readCKmeas = startck();                         // trigger CK101 measurement
          
    }
       
    if (readCKmeas){                                      // flag to read data only once per measurement
      if(currentMillis - ck101t_read > y){                // wait y milliseconds after triggering to read data, allow time for CK101 to finish measurement
                                                          // typically takes 60 seconds = 35 - 55 seconds for inflation and deflation, followed by 
                                                          // up to 5 seconds to update data in breakout board memory
        Serial.write(0x7e);
        readCKmeas = false;                               // flag to read data made false after reading
      }
    } 
    
}


/*  Case 1: Correct setup = readings displayed on lcd accurately. Occasionally one or more readings different than that displayed on sensor screen.
 *  May need averaging of muliple measurements to trigger alarm
 *  Case 2: CK101 not being powered, but breakout board is powered = displays abnormal high values, probably from memory - BPH:170, BPL:112, HB: 166
 *  Case 3: CK101 not setup correctly, cuff is loose, not inflating = displays previous valid measurement on lcd 
 * 
 */
