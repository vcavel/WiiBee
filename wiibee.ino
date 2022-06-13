/*
   -------------------------------------------------------------------------------------
   HX711_ADC
   Arduino library for HX711 24-Bit Analog-to-Digital Converter for Weight Scales
   Olav Kallhovd sept2017
   -------------------------------------------------------------------------------------
*/

/*
   Settling time (number of samples) and data filtering can be adjusted in the config.h file
   For calibration and storing the calibration value in eeprom, see example file "Calibration.ino"

   The update() function checks for new data and starts the next conversion. In order to acheive maximum effective
   sample rate, update() should be called at least as often as the HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS.
   If you have other time consuming code running (i.e. a graphical LCD), consider calling update() from an interrupt routine,
   see example file "Read_1x_load_cell_interrupt_driven.ino".

   This is an example sketch on how to use this library
*/

#include <HX711_ADC.h>

#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

//pins:
const int HX711_sck = D1; //mcu > HX711 sck pin
const int HX711_dout[4] = {D2, D3, D4, D5}; //mcu > HX711 dout pin

//HX711 constructor:
HX711_ADC loadCells[4] = {
    HX711_ADC(HX711_dout[0], HX711_sck),
    HX711_ADC(HX711_dout[1], HX711_sck),
    HX711_ADC(HX711_dout[2], HX711_sck),
    HX711_ADC(HX711_dout[3], HX711_sck)  
};

const int nbLoadCells = 4;
const int tareOffsetVal_eepromAdresses[4] = {0, 8, 16, 24};
const int calibrationFactorAdress = 32;
float calibrationFactor = 100.0;
unsigned long t = 0;

void setup() {
  Serial.begin(57600); 
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  restoreSettingsFromEeprom();  
}


void loop() {
  
  const int serialPrintInterval = 5000; //increase value to slow down serial print activity
  const int nbSamples = 5; // The more you get, the more accurate it is (but it is longer))
  int sum = 0;
  
  if (millis() > t + serialPrintInterval) {


    for(int i = 0; i < nbSamples; i++) {
      sum += readWeight();
    }
    Serial.print("Poids = ");
    Serial.print(sum / nbSamples);
    Serial.println("Kg");
    t = millis();
  }
  readSerialInput();
}

void readSerialInput() {
   
  // receive command from serial terminal
  // send 't' to initiate tare operation:
  // send 'c' to initiate calibration
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') refreshOffsetValueAndSaveToEEprom();
    if (inByte == 'c') calibrate();
  }
}
