/*

*/

#include "Arduino.h"
#include "MyCLass.h"

MyClass::MyClass(int pin)
{
  pinMode(pin, OUTPUT);
  _pin = pin;
}

// properties
int MyClass::pin() {
  return _pin;
}

void MyClass::pin(int pin) {
  _pin = pin;
}



