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
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" No finger?");
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
