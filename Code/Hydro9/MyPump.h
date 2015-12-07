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
    bool _pumpState;

    bool _pumpSchedule = false;
    
    unsigned long _pumpLastAction = 0; // last action millis

    int _pumpRunCycle = 15; // numer of minute pump should run
    int _pumpStopCycle = 15; //numer of minutes pump should stop


    int _pumptScheduleHourStart = 7;
    int _pumpScheduleHourStop = 18;

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
    bool getState();
    void pumpOn();
    void pumpOff();
    bool pumpSwitch();
    void pumpCheck();

    // Properties Pump
    void pumpRunCycle(int minutes);
    int  pumpRunCycle();
    
    void pumpStopCycle(int minutes);
    int  pumpStopCycle();

};

#endif
