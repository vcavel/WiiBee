boolean areNewValueAvailable() {

  boolean newDataReady = true;
  int i;
  
  // check for new data/start next conversion:
  for (i=0; i < nbLoadCells; i++) {
    newDataReady = loadCells[i].update() || newDataReady;
  }

  return newDataReady;
}

float readLoadSensorsAverage() {
  float sum = 0;
  int i;
  int sensorValue;

  refreshAllLoadCells();
  
  for(i = 0; i < nbLoadCells; i++) {

    while(loadCells[i].update() == 0) {}
    sensorValue = loadCells[i].getData();
    sum += sensorValue;
  }
  return sum/nbLoadCells;
}

float readWeight() {
  return readLoadSensorsAverage()*calibrationFactor;
}

// zero offset value (tare), calculate and save to EEprom:
void refreshOffsetValueAndSaveToEEprom() {
  Serial.println("Calculating tare offset value...");
  int i;
  
  for (i = 0; i < nbLoadCells; i++) {

    long _offset;

    _offset = 0;

    loadCells[i].tare(); // calculate the new tare / zero offset value (blocking)
    _offset = loadCells[i].getTareOffset(); // get the new tare / zero offset value
    EEPROM.put(tareOffsetVal_eepromAdresses[i], _offset); // save the new tare / zero offset value to EEprom
    loadCells[i].setTareOffset(_offset); // set value as library parameter (next restart it will be read from EEprom)
    Serial.print("New tare offset value:");
    Serial.print(_offset);
    Serial.print(", saved to EEprom adr:");
    Serial.print(tareOffsetVal_eepromAdresses[i]);
    Serial.print(", for load cell");
    Serial.println(i);
  }
  
  #if defined(ESP8266) || defined(ESP32)
    EEPROM.commit();
  #endif
  
}

void refreshAllLoadCells() {
  //Refresh all datasets
  int i;
  for (i = 0; i < nbLoadCells; i++) {
    loadCells[i].refreshDataSet();
  }
  
}

void calibrate() {

  Serial.println("Calibration process starting, please an object on the scale and input its weight in Kg");


  float known_mass = 0;
  bool _resume = false;

  while (_resume == false) {
    readLoadSensorsAverage();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Object weight is: ");
        Serial.print(known_mass);
        Serial.println(" kg");
        _resume = true;
      }
    }
  }
  
  float loadSensorsAverage = readLoadSensorsAverage();
  calibrationFactor = known_mass/loadSensorsAverage;
  Serial.print("Reading : ");
  Serial.print(loadSensorsAverage);
  Serial.println(", should be 1.0");
  Serial.print("Calibration factor set to ");
  Serial.println(calibrationFactor);
    
  //Recording the new calibration factor in the EEPROM
  EEPROM.put(calibrationFactorAdress, calibrationFactor);
 
  #if defined(ESP8266) || defined(ESP32)
    EEPROM.commit();
  #endif    
}

void restoreSettingsFromEeprom() {
  #if defined(ESP8266)|| defined(ESP32)
    EEPROM.begin(512);
  #endif

  EEPROM.get(calibrationFactorAdress, calibrationFactor);
  Serial.print("Loading calibration factor from EEPROM : ");
  Serial.println(calibrationFactor);


  int i;
  
  for (i = 0; i < nbLoadCells; i++) {
    loadCells[i].begin();
    delay(10);  

    //restore the zero offset value from eeprom:
    long tare_offset = 0;
    EEPROM.get(tareOffsetVal_eepromAdresses[i], tare_offset); 
    loadCells[i].setTareOffset(tare_offset);
    boolean _tare = false; //set this to false as the value has been resored from eeprom

    unsigned long stabilizingtime = 5000; // precision right after power-up can be improved by adding a few seconds of stabilizing time
    
    loadCells[i].start(stabilizingtime, _tare);
    if (loadCells[i].getTareTimeoutFlag()) {
      Serial.print("Timeout, check MCU>HX711 wiring and pin designations for loadCell ");
      Serial.println(i);
      while (1);
    } else {
      loadCells[i].setCalFactor(500.0); // set calibration value (float)
      Serial.print("Startup is complete for load cell ");
      Serial.println(i);
    }
  }  
}
