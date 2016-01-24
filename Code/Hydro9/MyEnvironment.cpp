/*
  MyEnvironment.h - Class to manage Environment sensos
  Features:
    - Air Temperature Sensor
    - Air Humidity Sensor
    - Water Temperature Sensor
    - Reports when temp changes
    - Report every x minutes?
*/

 
 

#include "Arduino.h"
#include "MyEnvironment.h"
#include "HydroConfig.h"
#include <DHT.h> // DHT Lib
#include <DallasTemperature.h>
#include <OneWire.h>

// Constructor

  MyEnvironment::MyEnvironment():_oneWire(ONE_WIRE_BUS),_dallas(&_oneWire) {

  }
  
  void MyEnvironment::dhtInit(){
    Serial.print("Initializing DHT... ");
    //DHT _dht;
    _dht.setup(DHT_PIN);
    //delay(_dht.getMinimumSamplingPeriod());
    Serial.println("Done!");
  }
  
  void MyEnvironment::dallasInit(){
    Serial.println("Dallas Temperature IC Control Library Demo");

    // locate devices on the bus
    Serial.print("Locating devices...");
    _dallas.begin();
    Serial.print("Found ");
    Serial.print(_dallas.getDeviceCount(), DEC);
    Serial.println(" devices.");
  
    // report parasite power requirements
    Serial.print("Parasite power is: "); 
    if (_dallas.isParasitePowerMode()) Serial.println("ON");
    else Serial.println("OFF");
    
    if (!_dallas.getAddress(_waterThermometer, 0)) {
      Serial.println("Unable to find address for Device 0"); 
    }else {
      // show the addresses we found on the bus
      Serial.print("Device 0 Address: ");
      printAddress(_waterThermometer);
      Serial.println();
      // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
      _dallas.setResolution(_waterThermometer, 9);
      Serial.print("Device 0 Resolution: ");
      Serial.print(_dallas.getResolution(_waterThermometer), DEC); 
      Serial.println();
    }
  }
  
  bool MyEnvironment::check(){
    if((_lastCheck+(15UL * 60UL * 1000UL)) < millis() or (_lastCheck == 0) ){
        updateDht();
        updateDallas();
        _lastCheck = millis();
        return true;
    }else {
      return false;
    }
  }
  
  void MyEnvironment::status(){
    Serial.println("************* ENVIRONMENT STATUS *******************");
    Serial.print("Air temperature:");
    Serial.print(_airTemp);
    Serial.print(" Air humidity:");
    Serial.print(_airHum);
    Serial.println();
    Serial.print("Water temperature:");
    Serial.print(_waterTemp);
    Serial.println();
    Serial.println("****************************************************");
  }
  
  float MyEnvironment::getAirTemp(){
    return _airTemp;
  }
  
  float MyEnvironment::getAirHum(){
    return _airHum;
  }
  
  float MyEnvironment::getWaterTemp(){
    return _waterTemp;
  }
  
  
  // *********  PRIVATE
  
  void MyEnvironment::updateDht(){
    if( (_lastUpdate + _dht.getMinimumSamplingPeriod()) < millis() or (_lastUpdate == 0)){
      delay(_dht.getMinimumSamplingPeriod());
      _airTemp = _dht.getTemperature();
      _airHum = _dht.getHumidity();
      _lastUpdate = millis();
    }
  }
  
  void MyEnvironment::updateDallas(){
    _dallas.requestTemperatures();
    _waterTemp = _dallas.getTempC(_waterThermometer);
  }
  
  // function to print a device address
  void MyEnvironment::printAddress(DeviceAddress deviceAddress)
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
    }
  }
