/*
  MyEnvironment.h - Class to manage Environment sensos
  Features:
    - Air Temperature Sensor
    - Air Humidity Sensor
    - Water Temperature Sensor
    - Reports when temp changes
    - Report every x minutes?
*/

#ifndef MyEnvironment_h
#define MyEnvironment_h

#include "Arduino.h"
#include "HydroConfig.h"
#include <DHT.h> // DHT Lib

  class MyEnvironment {
    private:
      float _airTemp=0;
      float _airHum=0;
      float _waterTemp=0;
      
      DHT _dht;
    protected:
    public:
      MyEnvironment();  //Contructor
      void dhtInit();
      
      float getAirTemp();
      float getAirHum();
      float getWaterTemp();
  };

#endif
