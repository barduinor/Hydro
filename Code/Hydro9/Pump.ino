// Pump Operations

#define RELAY_PUMP  3  // Arduino Digital I/O pin number for relay pump
#define RELAY_ON 1
#define RELAY_OFF 0
#define PUMP_ID 1   // Id of the sensor child

#define PUMP_RUN_TIME 60000 // in miliseconds

bool pump_state;
unsigned long previousMillis = millis();
MyMessage msg_Pump(PUMP_ID, V_STATUS);


void pump_init() {
  pinMode(RELAY_PUMP, OUTPUT);
}

void pump_present() {
  present(PUMP_ID, S_BINARY, "Pump");
  pump_off();
}

void pump_send(bool state, bool ack) {
  send(msg_Pump.set(state), ack); // Send new state
}

void pump_receive(bool state) {
  state ? pump_on() : pump_off();
}

void pump_on() {
  digitalWrite(RELAY_PUMP, RELAY_ON); // Turn Pump On
  pump_state = true;
  pump_send(pump_state, false);
  previousMillis = millis();
}

void pump_off() {
  digitalWrite(RELAY_PUMP, RELAY_OFF); // Turn Pump Off
  pump_state = false;
  pump_send(pump_state, false);
  previousMillis = millis();
}

void pump_switch() {
  pump_send((pump_state ? false : true), true); // send update to controller with ack
}

void pump_check() {
  unsigned long currentMillis = millis();
  if((currentMillis - previousMillis) > PUMP_RUN_TIME){
    pump_switch();
    previousMillis = currentMillis;
  }
}
