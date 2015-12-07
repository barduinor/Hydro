#include <Time.h>  
#include <DS1302RTC.h>  // A  DS3231/DS3232 library
//#include <Wire.h>
//#include <SPI.h>

boolean timeReceived = false;
// Set pins:  CE, IO,CLK
DS1302RTC RTC(A3, A5, A4);

void rtc_init(){
  Serial.println("RTC module activated");
  Serial.println();
  delay(500);
  
  if (RTC.haltRTC()) {
    Serial.println("The DS1302 is stopped.  Please run the SetTime");
    Serial.println("example to initialize the time and begin running.");
    Serial.println();
  }
  if (!RTC.writeEN()) {
    Serial.println("The DS1302 is write protected. This normal.");
    Serial.println();
  }
  
  
  // the function to get the time from the RTC
  setSyncProvider(RTC.get);  

  // Request latest time from controller at startup
  requestTime();  
}

void rtc_check(){
  // Update display every second
  if ((millis()-lastUpdate) > 10000) {
    rtc_updateDisplay();  
    lastUpdate = millis();
  }
}

void rtc_set(unsigned long controllerTime){
  // Ok, set incoming time 
  Serial.print("Time value received: ");
  Serial.println(controllerTime);
  RTC.set(controllerTime); // this sets the RTC to the time from controller - which we do want periodically
  timeReceived = true;
}

void rtc_updateDisplay(){
  tmElements_t tm;
  if(! RTC.read(tm)){
    Serial.print(tm.Day);
    Serial.print("/");
    Serial.print(tm.Month);
    Serial.print("/");
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.print(" ");
    
    printDigits(tm.Hour);
    Serial.print(":");
    printDigits(tm.Minute);
    Serial.print(":");
    printDigits(tm.Second);
    Serial.println();
  } 
}

void printDigits(int digits){
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}



