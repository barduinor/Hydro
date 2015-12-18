/*
  MyPump.h - Class to manage the pump in Hydorponic System
  Features:
    - Turn pump on for x minutes (15 default)
    - Turn Pump off for x Minutes (15 default)
    - Run pump from x hours to y hours (8 to 18)
    - Run pump while lux is between x and y (day light)
*/

#include "Arduino.h"
#include "MyPump.h"
#include "HydroConfig.h"

#include <DS1302RTC.h>  // A  DS3231/DS3232 library

// Constructor
  MyPump::MyPump(int relayPumpPin): _RTC(MY_RTC_CE_PIN, MY_RTC_IO_PIN, MY_RTC_CLK_PIN)
  {
    _relayPumpPin = relayPumpPin;
    pinMode(relayPumpPin, OUTPUT);
    pumpOff();
    _pumpLastAction=0;
  }

  void MyPump::rtc_init() 
  {
    Serial.println("RTC module activated");
    Serial.println();
    //delay(500);
  
    if (_RTC.haltRTC()) {
      Serial.println("The DS1302 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    }
    if (!_RTC.writeEN()) {
      Serial.println("The DS1302 is write protected. This normal.");
      Serial.println();
    }
  
    // the function to get the time from the RTC
    setSyncProvider(_RTC.get);
    
    Serial.print("RTC Sync TieStatus: ");
    Serial.print(timeStatus());
    Serial.print(" timeSet ");
    Serial.print(timeSet);
    
    if(timeStatus() == timeSet)
      Serial.println(" Ok!");
    else
      Serial.println(" FAIL!");
  }

// RTC MEthods ***************************************

  void MyPump::rtc_set(unsigned long controllerTime)
  {
    // Ok, set incoming time
    Serial.print("Time value received: ");
    Serial.println(controllerTime);
    _RTC.set(controllerTime); // this sets the RTC to the time from controller - which we do want periodically
    _timeReceived = true;
    
  }

  String MyPump::currentDateTime()
  {
    //output "2015/01/01 20:34:45"
     time_t t = now();
    String out;
    out = String(year(t))+"/";
    out = out + String(month(t)<10?"0":"");
    out = out + String(month(t))+"/";
    out = out + String(day(t)<10?"0":"");
    out = out + String(day(t))+" ";
    out = out + String(hour(t)<10?"0":"");
    out = out + String(hour(t))+":";
    out = out + String(minute(t)<10?"0":"");
    out = out + String(minute(t))+":";
    out = out + String(second(t)<10?"0":"");
    out = out + String(second(t))+" ";
    //Serial.print("Unix Time:");
    //Serial.println(now.unixtime());
    return out;
  }


//Methods Pump *******************************************

  void MyPump::pumpOn()
  {
    digitalWrite(MY_PUMP_RELAY_PIN, HIGH);
    _isPumpOn=true;
    _pumpLastAction = millis();
    Serial.print(currentDateTime().c_str());
    Serial.println("PUMP ON");
  }

  void MyPump::pumpOff()
  {
    digitalWrite(MY_PUMP_RELAY_PIN, LOW);
    _isPumpOn=false;
    _pumpLastAction = millis();
    Serial.print(currentDateTime().c_str());
    Serial.println(" PUMP OFF");
  }

  bool MyPump::pumpSwitch()
  {
    if( _isPumpOn )
      pumpOff();
    else
      pumpOn();
    return _isPumpOn;
  }

  bool MyPump::isOn()
  {
    return _isPumpOn;
  }

  bool MyPump::pumpCheck()
  {
      switch(_runMode){
        case RUN_MODE_OFF    : if(_isPumpOn) pumpOff();
                              break;
        case RUN_MODE_NORMAL :cycleCheck();
                              break;
        case RUN_MODE_SCHEDULE: if(scheduleCheck() || _isPumpOn) cycleCheck();
                              break;
        case RUN_MODE_DAYLIGHT: if(dayLightCheck() || _isPumpOn) cycleCheck();
                              break;
      }
  }
  
  void MyPump::pumpStatus(){
    Serial.println("**************** PUMP STATUS ***********************");
    
    Serial.print("Pump mode is: ");
    switch(_runMode){
      case RUN_MODE_OFF:      Serial.print("Off");      break;
      case RUN_MODE_NORMAL:   Serial.print("Normal");   break;
      case RUN_MODE_SCHEDULE: Serial.print("Schedule"); break;
      case RUN_MODE_DAYLIGHT: Serial.print("Daylight"); break;
      
      default:Serial.print("Unknnown"); break;
    }
    Serial.println();
    
    Serial.print("Pump Schedule: ");
    Serial.print(" Start:");
    Serial.print(_pumpScheduleStart);
    Serial.print(" Current:");
    Serial.print(currentUnixDay());
    Serial.print(" Stop:");
    Serial.print(_pumpScheduleStop);
    Serial.println();
    
    Serial.print("Pump Daylight Start:");
    Serial.print(_pumpLuxStart);
    Serial.print(" Current:");
    Serial.print(currentLuxLevel());
    Serial.println();

    
    Serial.println("****************************************************");
  }
  
// PRIVATE MEthods Pump ********************************************

  unsigned long MyPump::timeToUnixDay(byte bHour, byte bMin, byte bSec){ // converts time to UnixDay
    return ( (bHour * 60UL * 60UL * 1000UL) +
             (bMin  * 60UL * 1000UL) +
             (bSec  * 1000UL)
           );
  }
  unsigned long MyPump::currentUnixDay(){
    time_t t = now();
    return timeToUnixDay(hour(t),minute(t),second(t));
  }
  
  int MyPump::currentLuxLevel(){
    return ((1023-analogRead(LIGHT_SENSOR_PIN))/10.23);
  }
  
  bool MyPump::cycleCheck(){
    bool retVal=false;
    // Normal Cycle
    if (_isPumpOn) // true for on
    {
      if((_pumpLastAction + (_pumpCycleOn*60UL*1000UL)<millis()))
      {
        pumpOff();
        retVal=true;
      }
    }
    else{
      if((_pumpLastAction + (_pumpCycleStop*60UL*1000UL)<millis()) or _pumpLastAction==0)
      {
        pumpOn();
        retVal=true;
      }
    }
    return retVal;
  }

  bool MyPump::scheduleCheck(){
    return (
             (_pumpScheduleStart <= currentUnixDay())
         and (_pumpScheduleStop  >= currentUnixDay())
//         and _isScheduleOn 
           );
  }
  
  bool MyPump::dayLightCheck(){
    return ( currentLuxLevel() >= _pumpLuxStart);
  }

// Properties Pump *************************************************
  void MyPump::pumpCycleRun(int minutes)
  {
    _pumpCycleOn = minutes;
  }
  int MyPump::pumpCycleRun()
  {
    return _pumpCycleOn;
  }

  void MyPump::pumpCycleStop(int minutes)
  {
    _pumpCycleStop = minutes;
  }
  int MyPump::pumpCycleStop()
  {
    return _pumpCycleStop;
  }

  void MyPump::pumpScheduleStart(byte bHour, byte bMin)
  {
    _pumpScheduleStart = timeToUnixDay(bHour,bMin,0);
  }
  unsigned long MyPump::pumpScheduleStart()
  {
    return _pumpScheduleStart;
  }

  void MyPump::pumpScheduleStop(byte bHour, byte bMin)
  {
    _pumpScheduleStop = timeToUnixDay(bHour,bMin,0);
  }
  unsigned long MyPump::pumpScheduleStop()
  {
    return _pumpScheduleStop;
  }
  
  void MyPump::mode(pump_run_mode runMode){
    _runMode = runMode;
  }
  pump_run_mode MyPump::mode(){
    return _runMode;
  }
  
  void MyPump::pumpLuxStart(int lux){
    _pumpLuxStart = lux;
  }
  int MyPump::pumpLuxStart(){
    return _pumpLuxStart;
  }


// Utils ************************************************************

