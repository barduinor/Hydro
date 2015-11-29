/*

*/
#ifndef MyClass_h
#define MyClass_h

#include "Arduino.h"

class MyClass
{
  public:
    MyClass(int pin);

    // properties
    int pin();
    void pin(int pin);

  private:
    int _pin;
};

#endif
