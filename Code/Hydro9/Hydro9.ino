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


// Skecth name and version
#define SKETCH_NAME "Hydro9"
#define SKETCH_VERSION "v0.1"

// Pin Assignments

#define BUTTON_PIN  4  // Arduino Digital I/O pin number for button 

Bounce debouncer = Bounce();
int oldValue = 0;
unsigned long lastUpdate=0;

void setup()
{
  // Setup the button
  pinMode(BUTTON_PIN, INPUT);
  // Activate internal pull-up
  digitalWrite(BUTTON_PIN, HIGH);

  // After setting up the button, setup debouncer
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5);

  // Initialize Pump
  pump_init();
  rtc_init();
  // Request latest time from controller at startup
  requestTime();  

}

void presentation()  {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);

  // Register all sensors to gw (they will be created as child devices)
  pump_present();
}

/*
*  Example on how to asynchronously check for new messages from gw
*/
void loop()
{
  debouncer.update();
  // Get the update value
  int value = debouncer.read();
  if (value != oldValue && value == 0) {
    pump_switch();
  }
  oldValue = value;
  
  pump_check();
  rtc_check();
}

void receive(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
    Serial.println("This is an ack from gateway");
  }

  if (message.type == V_STATUS) {
    // Change relay state
    pump_receive(message.getBool());

    // Write some debug info
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", New status: ");
    Serial.println(message.getBool());
  }
}

void receiveTime(unsigned long controllerTime) {
  rtc_set(controllerTime);
}

