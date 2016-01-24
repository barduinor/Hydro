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

// Constructor

  MyEnvironment::MyEnvironment() {

  }
  
  void MyEnvironment::dhtInit(){
    Serial.print("Initializing DHT... ");
    //DHT _dht;
    _dht.setup(DHT_PIN);
    //delay(_dht.getMinimumSamplingPeriod());
    Serial.println("Done!");
  }
  
  bool MyEnvironment::check(){
    if((_lastCheck+(15UL * 60UL * 1000UL)) < millis() or (_lastCheck == 0) ){
        updateDht();
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
  
