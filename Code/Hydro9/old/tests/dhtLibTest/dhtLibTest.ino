
#include <DHT.h> // DHT library
#include "MyEnvironment.h"

MyEnvironment myEnvironment;

//DHT dht;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //dht.setup(DHT_PIN)
  myEnvironment.dhtInit();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("In loop milis:");
  Serial.print(millis());

  Serial.println();
  delay(1000);

}
