/*
  MyPump.h - Class to manage the pump in Hydorponic System
  Features:
    - Turn pump on for x minutes (15 default)
    - Turn Pump off for x Minutes (15 default)
    - Run pump from x hours to y hours (8 to 18)
    - Run pump while lux is between x and y (day light)
*/

#ifndef MyPump_h
#define MyPump_h

#include "Arduino.h"
#include "HydroConfig.h"

#include <DS1302RTC.h>  // A  DS3231/DS3232 library

#define RELAY_PUMP  3  // Arduino Digital I/O pin number for relay pump

class MyPump {
  private:
    int _relayPumpPin;
    bool _timeReceived = false;
    
    bool _isPumpOn;  // True, Pump is Running
    
    bool _isCycleOn    = false; // True Normal Cycle On/Off is Running
    bool _isScheduleOn = false; // True Schedule timer on/on
    bool _isDayLightOn = false; // True Check

    unsigned long _pumpLastAction = 0; // last action millis

    int _pumpCycleOn   = 15; // numer of minute pump should run
    int _pumpCycleStop = 15; //numer of minutes pump should stop

    unsigned long _pumpScheduleStart = 28800000; // 8:00  in unix 24 hour
    unsigned long _pumpScheduleStop   = 68400000; // 18:00 in unix 24 hour
   
    DS1302RTC _RTC;//(int RTC_CE_PIN, int RTC_IO_PIN, int RTC_CLK_PIN);

    void printDigits(int digits);


  public:
    // Constructor
    MyPump(int relayPumpPin);

    //properties
    bool Schedule();
    void Schedule(bool scheduleState);

    // Methods RTC

    void rtc_init();
    void rtc_set(unsigned long controllerTime);
    String currentDateTime();

    // Methods Pump
    bool isOn();
    bool isCycleOn();
    bool isScheduleOn();
    bool isDayLightOn();
    
    void pumpOn();
    void pumpOff();
    bool pumpSwitch();
    bool pumpCheck(); // returns true if there was a change in status

    // Properties Pump
    void pumpCycleRun(int minutes);
    int  pumpCycleRun();
    
    void pumpCycleStop(int minutes);
    int  pumpCycleStop();
    
    void pumpScheduleStart(byte bhour, byte bmin);
    unsigned long pumpScheduleStart();
    
    void pumpScheduleStop(byte bhour, byte bmin);
    unsigned long pumpScheduleStopMin(); 

};

#endif
