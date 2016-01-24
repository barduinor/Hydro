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
#include "HydroConfig.h"
#include "MyEnvironment.h"
#include <DHT.h> // DHT Lib

// Constructor

  MyEnvironment::MyEnvironment(): _dht(){
    _airTemp=0;
  }
  
  void MyEnvironment::dhtInit(){
    Serial.print("Initializing DHT... ");
    //DHT _dht;
    _dht.setup(DHT_PIN);
    //delay(_dht.getMinimumSamplingPeriod());
    Serial.println("Done!");
  }
