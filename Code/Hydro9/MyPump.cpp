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

MyPump::MyPump(int relayPumpPin): _RTC(MY_RTC_CE_PIN, MY_RTC_IO_PIN, MY_RTC_CLK_PIN)
{
  _relayPumpPin = relayPumpPin;
  pinMode(relayPumpPin, OUTPUT);
  pumpOff();
}

//properties
bool MyPump::Schedule() {
  return _isScheduleOn;
};

void MyPump::Schedule(bool scheduleState) {
  _isScheduleOn = scheduleState;
};

void MyPump::rtc_init() {
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
  out = out + String(month(t))+"/";
  out = out + String(day(t))+" ";
  out = out + String(hour(t))+":";
  out = out + String(minute(t))+":";
  out = out + String(second(t))+" ";
  //Serial.print("Unix Time:");
  //Serial.println(now.unixtime());
  return out;
}
//Methods Pump

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
bool MyPump::isCycleOn()
{
  return _isCycleOn;
}
bool MyPump::isScheduleOn()
{
  return _isScheduleOn;
}
bool MyPump::isDayLightOn()
{
  return _isDayLightOn;
}

bool MyPump::pumpCheck()
{
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
    if((_pumpLastAction + (_pumpCycleStop*60UL*1000UL)<millis()))
    {
      pumpOn();
      retVal=true;
    }
  }
  return retVal;
}

// Properties Pump
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

void MyPump::pumpScheduleStart(byte bhour, byte bmin)
{
  _pumpScheduleStart = (bhour * 60 * 60 * 1000);
}
unsigned long MyPump::pumpScheduleStart()
{
  return _pumpScheduleStart;
}

void MyPump::pumpScheduleStop(byte bhour, byte bmin)
{
  _pumpScheduleStop = (bhour * 60 * 60 * 1000);
}
unsigned long MyPump::pumpScheduleStopMin()
{
  return _pumpScheduleStop;
}


