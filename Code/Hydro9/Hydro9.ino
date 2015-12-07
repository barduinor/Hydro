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

#include "HydroConfig.h"
#include "MyPump.h"

// Skecth name and version
#define SKETCH_NAME "Hydro9"
#define SKETCH_VERSION "v0.1"

// Pin Assignments

#define BUTTON_PIN  4  // Arduino Digital I/O pin number for button 

Bounce debouncer = Bounce();
int oldValue = 0;
unsigned long lastUpdate = 0;

MyMessage msgPump(MY_PUMP_ID, V_STATUS);

MyPump myPump(MY_PUMP_RELAY_PIN);
void setup()
{
  // Setup the button
  pinMode(BUTTON_PIN, INPUT);
  // Activate internal pull-up
  digitalWrite(BUTTON_PIN, HIGH);

  // After setting up the button, setup debouncer
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);


  myPump.rtc_init();
  // Request latest time from controller at startup
  requestTime();
  wait(1000);// Wait for time form controller
  Serial.print("Current time: ");
  Serial.println(myPump.currentDateTime().c_str());
  myPump.pumpRunCycle(1);
  myPump.pumpStopCycle(1);

}

void presentation()  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);

  // Register all sensors to gw (they will be created as child devices)
  present(MY_PUMP_ID, S_BINARY, "Pump");
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
  myPump.pumpCheck();
}

void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
    Serial.println("This is an ack from gateway");
  }

  if (message.type == V_STATUS) {
    // Change relay state
    if (message.getBool())
      myPump.pumpOn();
    else
      myPump.pumpOff();
      
    send(msgPump.set(myPump.getState()), false);
    
    // Write some debug info
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", New status: ");
    Serial.println(message.getBool());
  }
}

void receiveTime(unsigned long controllerTime)
{
  myPump.rtc_set(controllerTime);
}

