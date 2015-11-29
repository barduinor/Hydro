#include "MyClass.h"

MyClass xpto(10);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  xpto.pin(11);
  Serial.println(xpto.pin());
  delay(1000);
}
