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
#include <DallasTemperature.h>
#include <OneWire.h>

  class MyEnvironment {
    private:
      float _airTemp=0;
      float _airHum=0;
      float _waterTemp=0;
      unsigned long _lastCheck=0;
      unsigned long _lastUpdate=0;
      
      DHT _dht;
      OneWire _oneWire;
      DallasTemperature _dallas;
      DeviceAddress _waterThermometer;
      
      void updateDht();
      void updateDallas();
      
      
    protected:
      void printAddress(DeviceAddress deviceAddress);
    public:
      MyEnvironment();  //Contructor
      void dhtInit();
      void dallasInit();
      bool check();
      void status();
      float getAirTemp();
      float getAirHum();
      float getWaterTemp();
  };

#endif
