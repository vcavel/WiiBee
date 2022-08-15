void reloadSettingsFromEEprom() {
  
  #if defined(ESP8266)|| defined(ESP32)
    EEPROM.begin(sizeof(WiiBeeSettings));
  #endif
  
  EEPROM.get(SETTINGS_ADDRESS, wiiBeeSettings);

  if (isnan(wiiBeeSettings.calibrationFactor)) {
    wiiBeeSettings.calibrationFactor = 100.0;
  }
  
  Serial.println("Loading settings from EEPROM ...");
  displaySettings();
}

void saveSettingsToEEprom() {

  //Recording the settings in the EEPROM
  EEPROM.put(SETTINGS_ADDRESS, wiiBeeSettings);
 
  #if defined(ESP8266) || defined(ESP32)
    EEPROM.commit();
  #endif 
}

void displaySettings() {

  Serial.println("<<<<<< Printing settings >>>>>>");
 
  Serial.print("tareOffsetVal = ");
  for (int i = 0; i < NB_LOAD_CELLS; i++) {
    Serial.println(wiiBeeSettings.tareOffsetVal[i]);
  }

  Serial.print("calibrationFactor = ");
  Serial.println(wiiBeeSettings.calibrationFactor, 10);

  Serial.print("ssid = ");
  Serial.println(wiiBeeSettings.ssid);

  Serial.print("passphrase = ");
  Serial.println(wiiBeeSettings.passphrase);
  
  Serial.print("server = ");
  Serial.println(wiiBeeSettings.server);
  
  Serial.print("wiiBeeName = ");
  Serial.println(wiiBeeSettings.wiiBeeName);
}
