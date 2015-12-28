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

  typedef enum {
    RUN_MODE_OFF,
    RUN_MODE_NORMAL,
    RUN_MODE_SCHEDULE,
    RUN_MODE_DAYLIGHT
  } pump_run_mode;

class MyPump {
  private:
    int _relayPumpPin;
    bool _timeReceived = false;
    bool _isPumpOn;  // True, Pump is Running
    pump_run_mode _runMode;

    unsigned long _pumpLastAction = 0; // last action millis

    int _pumpCycleOn   = 15; // numer of minute pump should run
    int _pumpCycleStop = 15; //numer of minutes pump should stop

    unsigned long _pumpScheduleStart  = 28800000; // 8:00  in unix 24 hour
    unsigned long _pumpScheduleStop   = 68400000; // 18:00 in unix 24 hour
    
    int _pumpLuxStart = 70; // Value from Ligth sensor to keep cycle the pump
   
    DS1302RTC _RTC;//(int RTC_CE_PIN, int RTC_IO_PIN, int RTC_CLK_PIN);
    
    bool cycleCheck(); // True if we should turn on pump based on Cycle
    bool scheduleCheck(); // True if pump is supposed to be running based on timer
    bool dayLightCheck(); // true id pump is supposed to be running on daylight
    unsigned long timeToUnixDay(byte bHour, byte bMin, byte bSec); // converts time to UnixDay
    String unixDayToTimeString(unsigned long unixDay);
    unsigned long currentUnixDay(); // Returns the current unix 24 hour day in milisec
    

  public:
  // Constructor

    MyPump(int relayPumpPin);

  // Methods RTC

    void rtc_init();
    void rtc_set(unsigned long controllerTime);
    String currentDateTime();

  // Methods Pump
    bool isOn(); // Is pump running
    
    void pumpOn();
    void pumpOff();
    bool pumpSwitch();
    bool pumpCheck(); // returns true if there was a change in status
    
    void pumpStatus();

  // Properties Pump
    void pumpCycleRun(int minutes);
    int  pumpCycleRun();
    
    void pumpCycleStop(int minutes);
    int  pumpCycleStop();
    
    void pumpScheduleStart(byte bhour, byte bmin);
    String pumpScheduleStart();
    
    void pumpScheduleStop(byte bhour, byte bmin);
    String pumpScheduleStop(); 
    
    void pumpLuxStart(int lux);
    int pumpLuxStart();
    
    void mode(pump_run_mode runMode);
    pump_run_mode mode();
    
    int currentLuxLevel(); // returns current light level
};

#endif
