boolean areNewValueAvailable() {

  boolean newDataReady = true;
  int i;
  
  // check for new data/start next conversion:
  for (i=0; i < NB_LOAD_CELLS; i++) {
    newDataReady = loadCells[i].update() || newDataReady;
  }

  return newDataReady;
}

float readLoadSensorsAverage() {
  float sum = 0.00;
  int i;
  int sensorValue;

  refreshAllLoadCells();
  for(i = 0; i < NB_LOAD_CELLS; i++) {
    
    //loadCells[i].update();
    sensorValue = loadCells[i].getData();
    sum += sensorValue;
  }
  return sum / NB_LOAD_CELLS;
}

float readWeight() {
  return readLoadSensorsAverage() * wiiBeeSettings.calibrationFactor;
}

// zero offset value (tare), calculate and save to EEprom:
void refreshOffsetValueAndSaveToEEprom() {
  Serial.println("Calculating tare offset value...");
  int i;

  int newTareOffsetValues[4];
  
  for (i = 0; i < NB_LOAD_CELLS; i++) {

    loadCells[i].tare(); // calculate the new tare / zero offset value (blocking)
    newTareOffsetValues[i] = loadCells[i].getTareOffset(); // get the new tare / zero offset value
    loadCells[i].setTareOffset(newTareOffsetValues[i]); // set value as library parameter (next restart it will be read from EEprom)
    wiiBeeSettings.tareOffsetVal[i] = newTareOffsetValues[i];
    Serial.print("New tare offset value:");
    Serial.print(newTareOffsetValues[i]);
    Serial.print(", for load cell");
    Serial.println(i);
  }

  saveSettingsToEEprom();
}

void refreshAllLoadCells() {
  //Refresh all datasets
  int i;
  for (i = 0; i < NB_LOAD_CELLS; i++) {
    loadCells[i].refreshDataSet();
  }
  
}

void calibrate() {

  Serial.println("Calibration process starting, please an object on the scale and input its weight in Kg");


  float known_mass = 0.000;
  bool _resume = false;

  while (_resume == false) {
    //readLoadSensorsAverage();
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
  wiiBeeSettings.calibrationFactor = known_mass / loadSensorsAverage;
  Serial.print("Reading : ");
  Serial.print(loadSensorsAverage);
  Serial.print(", should be ");
  Serial.println(known_mass);
  Serial.print("Calibration factor set to ");
  Serial.println(wiiBeeSettings.calibrationFactor, 10);
    
  saveSettingsToEEprom();
  
}

void restoreSettingsFromEeprom() {
  
  reloadSettingsFromEEprom();

  int i;
  
  for (i = 0; i < NB_LOAD_CELLS; i++) {
    loadCells[i].begin();
    delay(10);  

    //restore the zero offset value from eeprom:
    loadCells[i].setTareOffset(wiiBeeSettings.tareOffsetVal[i]);
    boolean _tare = false; //set this to false as the value has been resored from eeprom

    unsigned long stabilizingtime = 1000; // precision right after power-up can be improved by adding a few seconds of stabilizing time
    
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
