ConfigManager configManager; 

String macToStr(const uint8_t* mac) { 
    String result; 
    for (int i = 0; i < 6; ++i) { 
        result += String(mac[i], 16); 
        if ((i % 2)&&(i<5))result += ':';
    } 
    return result; 
}

void wifi_setup(void)
{
  Serial.print("Trying to connect to: ");
  Serial.println(g_devCfg.ssid);
  lcd_print(0, "Connecting to:");
  lcd_print(1, g_devCfg.ssid);

  /* Try to use the last saved SSIDs and passwords to connect.
   * If not successful:
   *  - Create a softAP on nodeMCU, no password
   *  - Display localSSID (made out of the MAC address) and IP address
   *  - If user connects via web browser, send form to get username and password for the LAN
   *  - If user POSTs the data, use it to connect to Wifi.
   *  - If cnnection succeeds, 
   *      - inform to the peer's browser and on the LCD
   *      - save credentials to EEPROM
   *      - disconnect Soft AP
   *  - Else inform via browser and LCD
   */
  WiFi.setAutoConnect(0); 
  WiFi.begin(g_devCfg.ssid,g_devCfg.pwd);
  
  Serial.println("Trying to connect with previously used ssids' and passwords ...");
  byte numRetries = 0;

  while ((WiFi.status() != WL_CONNECTED) && (numRetries < 10))
  { 
    // Wait for 10 seconds for the Wi-Fi to connect
    delay(1000);
    Serial.print('.');
    numRetries++;
  }
  Serial.println('.');
  wl_status_t wifi_status = WiFi.status();
  
  if(WL_CONNECTED != wifi_status)
  {
    //Unable to connect with past credentials, use softAP to fetch ssid and password from user
    char apCharBuf[24];
    WiFi.macAddress(g_devCfg.mac); 
    String apName= macToStr(g_devCfg.mac);
    apName.toCharArray(apCharBuf,24);
    Serial.println("Requesting password over local hotspot");
    Serial.print("Created Hotspot with name ");
    Serial.println(apCharBuf);
    // on LCD, Display SSID and IP Address
    lcd_wifi_hotspot(apCharBuf);
    configManager.setAPName(apCharBuf); 
    delay(2000);
    configManager.reset();
    // When new data has been entered, we restart the device from inside the configManager Lib
  }
  else
  {
    //Successfuly connected to the local area network!
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
  }
}
