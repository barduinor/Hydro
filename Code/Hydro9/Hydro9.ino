/**


**/

// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

// Enabled repeater feature for this node
//#define MY_REPEATER_FEATURE

// Define Node ID
#define MY_NODE_ID 7

// INCLUDES //
#include <SPI.h>
#include <MySensor.h>
#include <Bounce2.h>
#include <Time.h>
#include <DS1302RTC.h>  // A  DS3231/DS3232 library
#include <DHT.h> // DHT library
#include <DallasTemperature.h>
#include <OneWire.h>

#include "HydroConfig.h"
#include "MyPump.h"
#include "MyEnvironment.h"

// Skecth name and version
#define SKETCH_NAME "Hydro9"
#define SKETCH_VERSION "v0.1"


Bounce debouncer = Bounce();
int oldValue = 0;
unsigned long lastUpdate = 0;

unsigned long light_last_update=0;

//present(MY_PUMP_ID, S_BINARY, "Pump Switch");
  MyMessage msgPump(MY_PUMP_ID, V_STATUS);
  
//present(MY_PUMPMODE_ID, S_SCENE_CONTROLLER, "Pump Mode");
  MyMessage msgPumpMode(MY_PUMPMODE_ID,V_SCENE_ON);
  
//present(MY_SCHEDULER_ID, S_CUSTOM, "Pump  Scheduler");
  MyMessage msgScheduleStart(MY_SCHEDULER_ID,V_VAR1);
  MyMessage msgScheduleStop(MY_SCHEDULER_ID,V_VAR2);
  
//present(MY_DAYLIGHT_ID, S_CUSTOM, "Pump Daylight");
  MyMessage msgLight(MY_DAYLIGHT_ID, V_LEVEL);
  MyMessage msgLightNow(MY_DAYLIGHT_ID, V_VOLUME);

//present(MY_PUMPTIME_ID, S_CUSTOM, "Pump Time");
  MyMessage msgTime(MY_PUMPTIME_ID,V_VAR1);
  
//present(MY_PUMPCYCLE_ID, S_CUSTOM, "Pump Cycle");
  MyMessage msgCycleOn(MY_PUMPCYCLE_ID,V_LEVEL);
  MyMessage msgCycleOff(MY_PUMPCYCLE_ID,V_VOLUME);
  
//present(MY_AIRSENSOR_ID, S_TEMP, "Air Sensor");
  MyMessage msgAirTemp(MY_AIRSENSOR_ID,V_TEMP);
  MyMessage msgAirHum(MY_AIRSENSOR_ID,V_HUM);
  
//present(MY_WATERSENSOR_ID, S_TEMP, "Water Sensor");
  MyMessage msgWaterTemp(MY_WATERSENSOR_ID,V_TEMP);

MyPump myPump(MY_PUMP_RELAY_PIN);
MyEnvironment myEnvironment;

void setup()
{
  //********************** CLASSES *******************
  myEnvironment.dhtInit();
  myEnvironment.dallasInit();
  myPump.rtc_init();
  
  // ********************* HARDWHERE *****************
  // Setup the button
  pinMode(BUTTON_PIN, INPUT);
  // Activate internal pull-up
  digitalWrite(BUTTON_PIN, HIGH);

  // After setting up the button, setup debouncer
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);
  
  
  
  // Request latest time from controller at startup
  
  requestTime();
  wait(2000);// Wait for time from controller
  
  myPump.pumpScheduleStart(7,33);
  myPump.pumpScheduleStop(17,30);
  
  myPump.pumpLuxStart(70);
  
  myPump.mode(RUN_MODE_DAYLIGHT);
  
  //reportStatus();  
}

void presentation()  {
  Serial.println("Starting presentation");
  
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);

  // Register all sensors to gw (they will be created as child devices)
  present(MY_PUMP_ID, S_BINARY, "Pump Switch");
  present(MY_PUMPCYCLE_ID, S_CUSTOM, "Pump Cycle");
  present(MY_PUMPMODE_ID, S_SCENE_CONTROLLER, "Pump Mode");
  present(MY_SCHEDULER_ID, S_CUSTOM, "Pump Scheduler");
  present(MY_DAYLIGHT_ID, S_CUSTOM, "Pump Daylight");
  present(MY_AIRSENSOR_ID, S_TEMP, "Air Sensor");
  present(MY_WATERSENSOR_ID, S_TEMP, "Water Sensor");
  present(MY_PUMPTIME_ID, S_CUSTOM, "Pump Time");
  
  Serial.println("End presentation");
}


void loop()
{
  debouncer.update();
  // Get the update value
  int value = debouncer.read();
  if (value != oldValue && value == 0) {
    send(msgPump.set(myPump.pumpSwitch()), false);
  }
  oldValue = value;
  
  if(myPump.pumpCheck() || myEnvironment.check())
  {
     reportStatus();
  }
  
  if (((light_last_update + (15L * 60L * 1000UL)) < millis()) or (light_last_update == 0)){
    Serial.print(myPump.currentDateTime().c_str());
    Serial.print("Light level: ");
    Serial.println(myPump.currentLuxLevel());
    light_last_update = millis();
    
    reportStatus();
  }
}

void receive(const MyMessage &message) {
  String myPayload = message.getString();
  // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
    Serial.println("This is an ack from gateway");
  }

  switch(message.sensor){
    case MY_PUMP_ID:      if (message.type == V_STATUS) {
                            if (message.getBool())
                              myPump.pumpOn();
                            else
                              myPump.pumpOff();
                          }
                          break;
                          
    case MY_PUMPCYCLE_ID: if (message.type == V_LEVEL) {
                            myPump.pumpCycleRun(message.getInt());
                          } 
                          if (message.type == V_VOLUME) {
                            myPump.pumpCycleStop(message.getInt());
                          } 
                          break;
                          
    case MY_PUMPMODE_ID:  if (message.type == V_SCENE_ON) {
                            myPump.mode((pump_run_mode)message.getInt());
                          } 
                          break;
                          
    case MY_SCHEDULER_ID: 
                          if (message.type == V_VAR1) {
                            myPump.pumpScheduleStart((byte)myPayload.substring(0,2).toInt(),(byte)myPayload.substring(3,5).toInt()) ;   
                          } 
                          if (message.type == V_VAR2) {
                            myPump.pumpScheduleStop((byte)myPayload.substring(0,2).toInt(),(byte)myPayload.substring(3,5).toInt()) ;   
                          } 
                          break;
                          
    case MY_DAYLIGHT_ID:  if (message.type == V_LEVEL) {
                            myPump.pumpLuxStart(message.getInt());
                          } 
                          
                          break;
    
    default:  break;
  }

//  //debug message
//  Serial.println();
//  Serial.print("********************************");
//  Serial.println();
//  Serial.print("Get String:");
//  Serial.print(message.getString());
//  Serial.println();
//  Serial.print("Get Boolean:");
//  Serial.print(message.getBool());
//  Serial.println();
//  Serial.print("Get Byte:");
//  Serial.print(message.getByte());
//  Serial.println();
//  Serial.print("Get Float:");
//  Serial.print(message.getFloat());
//  Serial.println();
//  Serial.print("Get Int:");
//  Serial.print(message.getInt());
//  Serial.println();
//  Serial.print("Get UInt:");
//  Serial.print(message.getUInt());
//  Serial.println();
//  Serial.print("Get Long:");
//  Serial.print(message.getLong());
//  Serial.println();
//  Serial.print("Get ULong:");
//  Serial.print(message.getULong());
//  Serial.println();
//  Serial.print("Get Command:");
//  Serial.print(message.getCommand());
//  Serial.println();
//  Serial.print("********************************");
//  Serial.println();
  reportStatus();
}

void receiveTime(unsigned long controllerTime)
{
  myPump.rtc_set(controllerTime);
}

void reportStatus(){
  myPump.pumpStatus();
  myEnvironment.status();
  
  send(msgPump.set(myPump.isOn()), false);
  
  send(msgPumpMode.set(myPump.mode()),false);
  
  send(msgScheduleStart.set(myPump.pumpScheduleStart().c_str()),false);
  send(msgScheduleStop.set(myPump.pumpScheduleStop().c_str()),false);
  
  send(msgLight.set(myPump.pumpLuxStart()),false);
  send(msgLightNow.set(myPump.currentLuxLevel()), false);
  
  send(msgCycleOn.set(myPump.pumpCycleRun()), false);
  send(msgCycleOff.set(myPump.pumpCycleStop()), false);
  
  send(msgTime.set(myPump.currentDateTime().c_str()), false);
  
  send(msgAirTemp.set(myEnvironment.getAirTemp(),1), false);
  send(msgAirHum.set(myEnvironment.getAirHum(),1), false);
  
  send(msgWaterTemp.set(myEnvironment.getWaterTemp(),1), false);

}

