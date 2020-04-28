LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

unsigned long previousLcdUpdateMillis = 0; 
const long lcdUpdateInterval = 2000; 

void lcdSetup(void)
{
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(6,0);
  lcd.print("iVMS");
  lcd.setCursor(0,1);
  lcd.print("Initializing...");
}

void lcd_fingerMissing(void)
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousLcdUpdateMillis >= lcdUpdateInterval)
  {
      previousLcdUpdateMillis = currentMillis;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" No finger?");
  }
}

void clearLCDLine(int line)
{               
  lcd.setCursor(0,line);
  for(int n = 0; n < 16; n++)
  {
    lcd.print(" ");
  }
}

void lcd_print(byte lineNum, char* startPtr)
{
  if((lineNum == 0) || (lineNum == 1))
  {
    clearLCDLine(lineNum);
    lcd.setCursor(0,lineNum);
    if(NULL != startPtr)
      lcd.print(startPtr);
  }
}

void lcd_wifi_hotspot(char* softApName)
{
  lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print("Connect to:");
  lcd.setCursor(0,1);
  lcd.print(softApName);
}

void refreshLcd(void)
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousLcdUpdateMillis >= lcdUpdateInterval)
  {
      previousLcdUpdateMillis = currentMillis;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Avg BPM=");
      lcd.print(max30102_getHeartRate());
      lcd.setCursor(0,1);
      lcd.print("spO2=");
      lcd.print(max30102_getSpO2());
  }
}
