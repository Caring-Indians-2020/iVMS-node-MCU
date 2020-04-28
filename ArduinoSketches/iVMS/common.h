
void wifi_setup(void);
void lcdSetup(void);
void lcd_fingerMissing(void);
void refreshLcd(void);
int32_t max30102_getSpO2(void);
int32_t max30102_getHeartRate(void);
bool max30102_isDataValid(void);
void max30102_Init(void);
void max30102_Setup(void);
void max30102_printPPG_to_UART(void);
void max30102_measure(int32_t bufLen);
void lcd_wifi_hotspot(void);