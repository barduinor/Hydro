/*
    **************Billie's Hydroponic Controller V1.1.2b**************
 **************-------------------------------------***************
 ****Made by Tom De Bie for anyone who can find a use for it ;)****
 ******************************************************************
 ******************************************************************
 Software Requirements:
 ----------------------
 -Arduino IDE 1.0
 -UTFT library
 -ITDB02_Touch library
 -SD library
 -Wire library
 -RTClib library
 -EEPROM library
 -EEPROMex library
 
 Hardware Requirements:
 ----------------------
 -Arduino Mega 2560
 -3.2 inch TFT Touchscreen
 -LC Studio SD Cardreader
 -DS1307 Real Time Clock module
 -Phidgets 1130 pH/ORP module
 -4 channel Relay 220V/10Amp
 -DHT11 or DHT22 sensor
 -pH electrode probe with BNC connector
 -Photoresistor
 -Solenoid valve 12V
 -2x Peristaltic pump
 -2x Duckbill side waterlevel Float sensor
 -resistors (220ohm - 1Kohm)
 -Cermet Potentiometer 110Kohm
 -Protoboard
 -9V DC powersupply
 */

#include <UTFT.h>                  //16bit TFT screen library
#include <ITDB02_Touch.h>          //Touchscreen library
#include <SD.h>                    //SD card library
#include <Wire.h>                  //One Wire library
#include "RTClib.h"                //Real Time Clock library
#include <EEPROMex.h>              //Extended Eeprom library

UTFT myGLCD(ITDB32S,38,39,40,41);  //pins used for TFT
ITDB02_Touch  myTouch(6,5,4,3,2);  //pins used for Touch

#define dht_dpin 69                //pin for DHT11
int pHPin = 59;                    //pin for pH probe
int pHPlusPin = 45;                //pin for Base pump (relay)
int pHMinPin = 43;                 //pin for Acide pump (relay)
int ventilatorPin = 49;            //pin for Fan (relay)
int floatLowPin = 8;               //pin for lower float sensor
int floatHighPin = 7;              //pin for upper float sensor
int solenoidPin = 47;              //pin for Solenoid valve (relay)
int lightSensor = 60;              //pin for Photoresistor
const int chipSelect = 53;         //pin for chipselect SD card

extern uint8_t BigFont[];          //Which fonts to use...
extern uint8_t SmallFont[];
extern uint8_t SevenSegNumFont[];

RTC_DS1307 RTC;                    //Define RTC module

//****************************************************************//
//*********Declaring Variables************************************//
//****************************************************************//
int x, y;                          //x and y axis for touch
int page = 0;                      //returns which page your on
int tankProgState = 1;             //returns the state of tank program - on or off
float pH;                          //generates the value of pH
boolean smoothPh = 0;              //variable that sets smoothing of pH on or off

const int numReadings = 10;        
int readings[numReadings];         //the readings from the analog input
int index = 0;                     //the index of the current reading
int total = 0;                     //the running total
int average = 0;                   //the average

int count=0;

int ledState = LOW;                //variables for pulsing the pump
long previousMillis = 0;           //             |
long pinHighTime = 100;            //             |
long pinLowTime = 7500;            //             |
long pinTime = 100;                //             |

int sdState = LOW;                 //variables for delayed writing to SD card
long sdPreviousMillis = 0;         //             |
long sdTime = 7500;                //             |

int pmem = 0;                      //check which page your on
float Setpoint;                    //holds value for Setpoint
float HysterisMin;                 //Minimum deviation from Setpoint
float HysterisPlus;                //Maximum deviation from Setpoint
float SetHysteris;                 //Holds the value for Hysteris
float FanTemp;                     //Holds the set value for temperature
float FanHumid;                    //Holds the set value for humidity
float fanHysteris = 2;             //Set value for hysteris tuning Fan

int lightADCReading;               //variables for measuring the light
double currentLightInLux;          //              |
double lightInputVoltage;          //              |
double lightResistance;            //              |

int EepromSetpoint = 10;           //location of Setpoint in Eeprom
int EepromSetHysteris = 20;        //location of SetHysteris in Eeprom
int EepromFanTemp = 40;            //location of FanTemp in Eeprom
int EepromFanHumid = 60;           //location of FanHumid in Eeprom
int EepromPinHighTime = 80;        //location of pinHighTime in Eeprom
int EepromPinLowTime = 100;        //location of pinLowTime in Eeprom
int EepromSmoothPh = 90;          //location of smoothPh in Eeprom

DateTime now;                      //call current Date and Time

#define DHTTYPE DHT11              //define which DHT chip is used - DHT11 or DHT22
byte bGlobalErr;                   //for passing error code back.
byte dht_dat[4];                   //Array to hold the bytes sent from sensor.


void setup()                   //Every block is in a separate function. 
{
  EepromRead();                //Reads every value that is stored in Eeprom
  smoothArraySetup();          //Sets the array for smoothing the pH value to 0
  logicSetup();                //Replaces the void Setup
  graphSetup();                //Initialises the TFT screen
  timeSetup();                 //Initialises the RTC module
  SDSetup();                   //Initialises the SD module

}

void loop()
{

  graphLoop();                //Loop for TFT screen
  logicLoop();                //pH algorithm loops through this one, also the smooting of the signal
  fotoLoop();                 //Light measurements loop through this one
  FanControl();               //Fans are controlled in this loop
  TankProgControl();          //Conrolling loop for refilling the tank
  SDLoop();                   //Writing all sensor data to SD
}


void EepromRead()            //reads eepromvalue for following variables
{
  Setpoint = EEPROM.readFloat(EepromSetpoint);        
  SetHysteris = EEPROM.readFloat(EepromSetHysteris);
  FanTemp = EEPROM.read(EepromFanTemp);
  FanHumid = EEPROM.read(EepromFanHumid);
  pinHighTime = EEPROM.readLong(EepromPinHighTime);
  pinLowTime = EEPROM.readLong(EepromPinLowTime);
  smoothPh = EEPROM.readInt(EepromSmoothPh);
}

// Initial setup
void graphSetup()                 //initialises the lcd screen
{        
  myGLCD.InitLCD(LANDSCAPE);      //LANDSCAPE or PORTRAIT
  myGLCD.clrScr();
  myTouch.InitTouch(LANDSCAPE);   //LANDSCAPE or PORTRAIT
  myTouch.setPrecision(PREC_HI);  //Set precision
  mainscr();                      //Default screen is mainscr
  page = 0;                       //Set page to 0
}


void smoothArraySetup()
{
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
  {
    readings[thisReading] = 0;
  }
}


void logicSetup()
{

  pinMode(pHPlusPin, OUTPUT);
  pinMode(pHMinPin, OUTPUT);
  pinMode(ventilatorPin, OUTPUT);
  pinMode(solenoidPin, OUTPUT);

  pmem==0;

  InitDHT();
  delay(300);
}


void logicLoop()
{
  ReadDHT();                                        //call DHT chip for data

  if (smoothPh == 0)                                //If smoothPh = 0 then no smooting is used
  {                                                 //                  |
    float sensorValue = 0;                          //Default the Value = 0
    sensorValue = analogRead(pHPin);                //sensorValue gets the value from the pH probe
    pH = (0.0178 * sensorValue - 1.889);            //pH = the calculated value
                                                    //                  |
    HysterisMin = (Setpoint - SetHysteris);         //HysterisMin = the lowest value that is allowed by the program. Lower then this and a Base is added.
    HysterisPlus = (Setpoint + SetHysteris);        //HysterisPlus = the highest value that is allowed by the program. Higher and an acid id added.
                                                    //                  |
    if (pmem == 0)                                  //If pmem equals 0, then goto the next if statement.                 
    {                                               //                  |
      if (pH < HysterisMin)                         //If pH is smaller then HysterisMin, then set pmem to 1
      {                                             //                  |
        pmem = 1;                                   //                  |
      }                                             //                  |
                                                    //                  |
      if (pH >= HysterisMin && pH <= HysterisPlus)  //If pH is greater or the same as HysterisMin AND pH is smaller of the same as HysterisPlus, then do nothing.
      {                                             //                  |
        digitalWrite (pHPlusPin, LOW);              //Set base pump to off position
        digitalWrite (pHMinPin, LOW);               //Set acid pump to off position
      }                                             //                  |
                                                    //                  |
      if (pH > HysterisPlus)                        //If pH is greater then HysterisPlus, set pmem to 2
      {                                             //                  |
        pmem = 2;                                   //                  |
      }                                             //                  |
    }                                               //                  |
                                                    //                  |
                                                    //                  |
    if (pmem == 1)                                  //If pmem equals 1, the goto next if statement
    {                                               //                  |
      if (pH < HysterisMin)                         //If pH is smaller then HysterisMin, then 
      {
        unsigned long currentMillis = millis();
        if(currentMillis - previousMillis > pinTime)
        {
          previousMillis = currentMillis;

          if (ledState == LOW)
          {
            ledState = HIGH;
            pinTime = pinHighTime;
          }
          else
          {
            ledState = LOW;
            pinTime = pinLowTime;
          }
          digitalWrite (pHPlusPin, ledState);
          digitalWrite (pHMinPin, LOW);
        }      
      }

      if (pH >= HysterisMin && pH < Setpoint)
      {
        unsigned long currentMillis = millis();
        if(currentMillis - previousMillis > pinTime)
        {
          previousMillis = currentMillis;

          if (ledState == LOW)
          {
            ledState = HIGH;
            pinTime = pinHighTime;
          }
          else
          {
            ledState = LOW;
            pinTime = pinLowTime;
          }
          digitalWrite (pHPlusPin, ledState);
          digitalWrite (pHMinPin, LOW);
        }      
      }

      if (pH >= Setpoint)
      {
        pmem = 0;
      }
    }  

    if (pmem == 2)
    {
      if (pH > HysterisPlus)
      {
        unsigned long currentMillis = millis();
        if(currentMillis - previousMillis > pinTime)
        {
          previousMillis = currentMillis;

          if (ledState == LOW)
          {
            ledState = HIGH;
            pinTime = pinHighTime;
          }
          else
          {
            ledState = LOW;
            pinTime = pinLowTime;
          }
          digitalWrite (pHMinPin, ledState);
          digitalWrite (pHPlusPin, LOW);
        }      
      }

      if (pH <= HysterisPlus && pH > Setpoint)
      {
        unsigned long currentMillis = millis();
        if(currentMillis - previousMillis > pinTime)
        {
          previousMillis = currentMillis;

          if (ledState == LOW)
          {
            ledState = HIGH;
            pinTime = pinHighTime;
          }
          else
          {
            ledState = LOW;
            pinTime = pinLowTime;
          }
          digitalWrite (pHMinPin, ledState);
          digitalWrite (pHPlusPin, LOW);
        }      
      }

      if (pH <= Setpoint)
      {
        pmem = 0;
      }
    }  
  }   
  if (smoothPh == 1)
  {
    total = total - readings[index];
    readings[index] = analogRead(pHPin);
    total = total + readings[index];
    index = index + 1;

    if (index >= numReadings)
    {
      index = 0;
    }

    average = total / numReadings;

    float sensorValue = 0;
    sensorValue = average;
    pH = (0.0178 * sensorValue - 1.889);

    HysterisMin = (Setpoint - SetHysteris);
    HysterisPlus = (Setpoint + SetHysteris);

    ++count;
    if (count > 10)
    {
      count = 10;
    }

    if (count == 10)
    {  
      if (pmem == 0)
      {
        if (pH < HysterisMin)
        {
          pmem = 1;
        }

        if (pH >= HysterisMin && pH <= HysterisPlus)
        {
          digitalWrite (pHPlusPin, LOW);
          digitalWrite (pHMinPin, LOW);
        }

        if (pH > HysterisPlus)
        {
          pmem = 2;
        }
      }


      if (pmem == 1)
      {
        if (pH < HysterisMin)
        {
          unsigned long currentMillis = millis();
          if(currentMillis - previousMillis > pinTime)
          {
            previousMillis = currentMillis;

            if (ledState == LOW)
            {
              ledState = HIGH;
              pinTime = pinHighTime;
            }
            else
            {
              ledState = LOW;
              pinTime = pinLowTime;
            }
            digitalWrite (pHPlusPin, ledState);
            digitalWrite (pHMinPin, LOW);
          }      
        }

        if (pH >= HysterisMin && pH < Setpoint)
        {
          unsigned long currentMillis = millis();
          if(currentMillis - previousMillis > pinTime)
          {
            previousMillis = currentMillis;

            if (ledState == LOW)
            {
              ledState = HIGH;
              pinTime = pinHighTime;
            }
            else
            {
              ledState = LOW;
              pinTime = pinLowTime;
            }
            digitalWrite (pHPlusPin, ledState);
            digitalWrite (pHMinPin, LOW);
          }      
        }

        if (pH >= Setpoint)
        {
          pmem = 0;
        }
      }  

      if (pmem == 2)
      {
        if (pH > HysterisPlus)
        {
          unsigned long currentMillis = millis();
          if(currentMillis - previousMillis > pinTime)
          {
            previousMillis = currentMillis;

            if (ledState == LOW)
            {
              ledState = HIGH;
              pinTime = pinHighTime;
            }
            else
            {
              ledState = LOW;
              pinTime = pinLowTime;
            }
            digitalWrite (pHMinPin, ledState);
            digitalWrite (pHPlusPin, LOW);
          }      
        }

        if (pH <= HysterisPlus && pH > Setpoint)
        {
          unsigned long currentMillis = millis();
          if(currentMillis - previousMillis > pinTime)
          {
            previousMillis = currentMillis;

            if (ledState == LOW)
            {
              ledState = HIGH;
              pinTime = pinHighTime;
            }
            else
            {
              ledState = LOW;
              pinTime = pinLowTime;
            }
            digitalWrite (pHMinPin, ledState);
            digitalWrite (pHPlusPin, LOW);
          }      
        }

        if (pH <= Setpoint)
        {
          pmem = 0;
        }
      }  
    } 
  }


  if (page == 0)
  {
    myGLCD.printNumF(pH, 2, 91, 23);
    myGLCD.printNumI(dht_dat[0], 91, 115, 3);
    myGLCD.printNumI(dht_dat[2], 91, 69, 3);
    myGLCD.printNumI(currentLightInLux, 91, 162, 4);
  }
  delay(250);               
}


void graphLoop()
{
  if (true)
  {
    if (myTouch.dataAvailable())

    {
      myTouch.read();
      x=myTouch.getX();
      y=myTouch.getY();

      if (page == 0)
      {
        if ((x>=255) && (x<=312))
        {
          if ((y>=17) && (y<=47))
          {
            //action for 'SET' button pH
            page = 2;
            myGLCD.clrScr();
            PhSetting();
          }
          if ((y>=86) && (y<=114))
          {
            //action for 'SET' button fans.
            page = 1;
            myGLCD.clrScr();
            FanSetting();
          }
          if ((y>=201) && (y<=229))
          {
            //action for 'SET' button Tank.
            page = 3;
            myGLCD.clrScr();
            TankControl();
          }
        }
      }

      if (page == 1)
      {
        if ((x>=155) && (x<=182))
        {
          if ((y>=47) && (y<=75))
          {
            //action for plus sign temp.
            FanIncreaseTemp();
          }
          if ((y>=96) && (y<=122))
          {
            //action for minus sign temp.
            FanDecreaseTemp();
          }
          if ((y>=131) && (y<=159))
          {
            //action for plus sign humidity.
            FanIncreaseHumid();
          }
          if ((y>=180) && (y<=206))
          {
            //action for minus sign humidity.
            FanDecreaseHumid();
          }           
        }
        if ((x>=235) && (x<=307))
        {
          if ((y>=42) && (y<=71))
          {
            //action for 'save' button
            page = 0;
            myGLCD.clrScr();
            mainscr();
            EEPROM.write(EepromFanTemp, FanTemp);
            EEPROM.write(EepromFanHumid, FanHumid);
          }
        }
        if ((x>=207) && (x<=307))
        {
          if ((y>=155) && (y<=184))
          {
            //action for 'cancel' button
            page = 0;
            myGLCD.clrScr();
            mainscr();
            FanTemp = EEPROM.read(EepromFanTemp);
            FanHumid = EEPROM.read(EepromFanHumid);
          }
        }
      }
      if (page == 2)
      {
        if ((x>=155) && (x<=182))
        {
          if ((y>=47) && (y<=75))
          {
            //action for plus sign pHmin.
            phIncreaseSetpoint();
          }
          if ((y>=96) && (y<=122))
          {
            //action for minus sign pHmin.
            phDecreaseSetpoint();
          }
          if ((y>=131) && (y<=159))
          {
            //action for plus sign pHplus.
            phIncreaseHysteris();
          }
          if ((y>=180) && (y<=206))
          {
            //action for minus sign pHplus.
            phDecreaseHysteris();
          }           
        }
        if ((x>=235) && (x<=307))
        {
          if ((y>=42) && (y<=71))
          {
            //action for 'save' button
            page = 0;
            myGLCD.clrScr();
            mainscr();
            EEPROM.writeFloat(EepromSetpoint, Setpoint);
            EEPROM.writeFloat(EepromSetHysteris, SetHysteris);
            EEPROM.writeLong(EepromPinHighTime, pinHighTime);
            EEPROM.writeLong(EepromPinLowTime, pinLowTime);
            EEPROM.writeInt(EepromSmoothPh, smoothPh);
          }
        }
        if ((x>=207) && (x<=307))
        {
          if ((y>=155) && (y<=184))
          {
            //action for 'cancel' button
            page = 0;
            myGLCD.clrScr();
            mainscr();
            Setpoint = EEPROM.readFloat(EepromSetpoint);
            SetHysteris = EEPROM.readFloat(EepromSetHysteris);
            pinHighTime = EEPROM.readLong(EepromPinHighTime);
            pinLowTime = EEPROM.readLong(EepromPinLowTime);
            smoothPh = EEPROM.readInt(EepromSmoothPh);
          }
        }
        if ((x>=300) && (x<=315))
        {
          if ((y>=220) && (y<=235))
          {
            //action for next '>' button
            page = 4;
            myGLCD.clrScr();
            PhSetting2();
          }
        }
      }
      if (page == 3)
      {
        if ((x>=45) && (x<=167))
        {
          if ((y>=64) && (y<=113))
          {
            //action for 'En/Disable control' button
          }
        }
        if ((x>=25) && (x<=225))
        {
          if ((y>=164) && (y<=194))
          {
            //action for 'Manual Refil' button.
            ManualRefilProg();
          }
        }
        if ((x>=225) && (x<=297))
        {
          if ((y>=72) && (y<=101))
          {
            //action for 'Home' button.
            page = 0;
            myGLCD.clrScr();
            mainscr();
          }
        }
      }
      if (page == 4)
      {
        if ((x>=155) && (x<=182))
        {
          if ((y>=47) && (y<=75))
          {
            //action for plus sign Pump On.
            IncreasePumpHighTime();
          }
          if ((y>=96) && (y<=122))
          {
            //action for minus sign Pump On.
            DecreasePumpHighTime();
          }
          if ((y>=131) && (y<=159))
          {
            //action for plus sign Pump Off.
            IncreasePumpLowTime();
          }
          if ((y>=180) && (y<=206))
          {
            //action for minus sign pump Off.
            DecreasePumpLowTime();
          }           
        }
        if ((x>=200) && (x<=260))
        {
          if ((y>=105) && (y<=155))
          {
            //action for 'on' button
            smoothPh = 1;
          }
        }
        if ((x>=261) && (x<=307))
        {
          if ((y>=105) && (y<=155))
          {
            //action for 'off' button
            smoothPh = 0;
          }
        }
        if ((x>=10) && (x<=25))
        {
          if ((y>=220) && (y<=235))
          {
            //action for previous '<' button   10, 220, 25, 235
            page = 2;
            myGLCD.clrScr();
            PhSetting();
          }
        }
      }
    }
  }
}


//Mainmenu
void mainscr()
{
  myGLCD.fillScr(0, 0, 0);
  myGLCD.setBackColor (0, 0, 0);

  myGLCD.setFont(SmallFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor (0, 0, 0);
  myGLCD.print("pH", 25, 23);
  myGLCD.print("Temp", 25, 69);
  myGLCD.print("Humid", 25, 115);
  myGLCD.print("Light", 25, 161);
  myGLCD.print("Tank", 25, 207);
  myGLCD.print("Fans", 200, 92);
  myGLCD.print("C", 150, 71); //degree celcius
  myGLCD.print("%", 150, 117); //Percent
  myGLCD.print("Lux.", 165, 163); //Lux
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawLine(160, 79, 196, 97);
  myGLCD.drawLine(160, 120, 196, 97);

  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (255, 17, 312, 47);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("SET", 260, 23); //set pH
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (255, 86, 312, 114);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("SET", 260, 92); //set Fans
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (255, 201, 312, 229);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("SET", 260, 207); //set Tank

  myGLCD.setFont(BigFont);
  myGLCD.setColor(0, 0, 255);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect (86, 17, 173, 47);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawRoundRect (86, 17, 173, 47);
  myGLCD.setColor(0, 0, 255);
  //myGLCD.print("5.0", 91, 23); //location value pH
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect (86, 64, 143, 93);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawRoundRect (86, 64, 143, 93);
  myGLCD.setColor(0, 0, 255);
  //myGLCD.print("25", 91, 69); //location value Temp
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect (86, 110, 143, 139);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawRoundRect (86, 110, 143, 139);
  myGLCD.setColor(0, 0, 255);
  //myGLCD.print("100", 91, 115); //location value Humid
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect (86, 156, 158, 185);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawRoundRect (86, 156, 158, 185);
  myGLCD.setColor(0, 0, 255);
  //myGLCD.print("2200", 91, 161); //location value Light
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect (86, 202, 222, 231);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawRoundRect (86, 202, 222, 231);
  myGLCD.setColor(0, 0, 255);
  //myGLCD.print("disabled", 91, 207); //location value Tank
}

//Fan Settings - page 1
void FanSetting()
{
  myGLCD.fillScr(0, 0, 0);
  myGLCD.setBackColor (0, 0, 0);

  myGLCD.setFont(SmallFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor (0, 0, 0);
  myGLCD.print("Temp", 25, 79);
  myGLCD.print("Humid", 25, 165);
  myGLCD.print("C", 140, 79);
  myGLCD.print("%", 140, 165);

  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor (0, 0, 0);
  myGLCD.print("Fan Settings", CENTER, 0);

  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor (255, 255, 255);
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect (71, 72, 128, 101);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawRoundRect (71, 72, 128, 101);
  myGLCD.setColor(0, 0, 255);
  myGLCD.printNumI(FanTemp, 76, 79); //value Temp
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect (71, 155, 128, 184);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawRoundRect (71, 155, 128, 184);
  myGLCD.setColor(0, 0, 255);
  myGLCD.printNumI(FanHumid, 76, 162); //value Humid

  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (155, 47, 182, 75);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("+", 160, 53);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (155, 96, 182, 122);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("-", 160, 102);

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (155, 131, 182, 159);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("+", 160, 137);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (155, 180, 182, 206);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("-", 160, 186);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawLine(167, 78, 167, 95);
  myGLCD.drawLine(168, 78, 168, 95);
  myGLCD.drawLine(167, 162, 167, 179);
  myGLCD.drawLine(168, 162, 168, 179);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (235, 42, 307, 71);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Save", 240, 49);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (207, 155, 307, 184);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Cancel", 210, 162);
}


//pH Settings - page 2
void PhSetting()
{
  myGLCD.fillScr(0, 0, 0);
  myGLCD.setBackColor (0, 0, 0);

  myGLCD.setFont(SmallFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor (0, 0, 0);
  myGLCD.print("Setp.", 25, 79);
  myGLCD.print("His.", 25, 165);

  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor (0, 0, 0);
  myGLCD.print("pH Settings", CENTER, 0);

  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor (255, 255, 255);
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect (71, 72, 143, 101);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawRoundRect (71, 72, 143, 101);
  myGLCD.setColor(0, 0, 255);
  myGLCD.printNumF(Setpoint, 2, 76, 79); //value Min. pH
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect (71, 155, 143, 184);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawRoundRect (71, 155, 143, 184);
  myGLCD.setColor(0, 0, 255);
  myGLCD.printNumF(SetHysteris, 2, 76, 162); //value Max. pH

  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (155, 47, 182, 75);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("+", 160, 53);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (155, 96, 182, 122);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("-", 160, 102);

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (155, 131, 182, 159);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("+", 160, 137);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (155, 180, 182, 206);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("-", 160, 186);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawLine(167, 78, 167, 95);
  myGLCD.drawLine(168, 78, 168, 95);
  myGLCD.drawLine(167, 162, 167, 179);
  myGLCD.drawLine(168, 162, 168, 179);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (235, 42, 307, 71);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Save", 240, 49);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (207, 155, 307, 184);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Cancel", 210, 162);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (296, 218, 315, 235);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print(">", 298, 220);
}

//Manual Tank Control - page 3
void TankControl()
{
  myGLCD.fillScr(0, 0, 0);
  myGLCD.setBackColor (0, 0, 0);

  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor (0, 0, 0);
  myGLCD.print("Manual Tank Control", CENTER, 0);

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (225, 72, 297, 101);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor (0, 0, 255);
  myGLCD.print("Home", 230, 79);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (45, 64, 167, 113);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor (0, 0, 255);
  myGLCD.print("Enable", 50, 71);
  myGLCD.print("Control", 50, 91);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (25, 164, 225, 194);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor (0, 0, 255);
  myGLCD.print("Manual Refil", 30, 171);

  myGLCD.setFont(BigFont);
  myGLCD.setColor(0, 0, 255);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect (257, 164, 310, 194);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawRoundRect (257, 164, 310, 194);
  myGLCD.setColor(0, 0, 255);
  myGLCD.print("OFF", 260, 171); //status refil
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawLine(229, 179, 253, 179);
}

//pH Settings2 - page 4
void PhSetting2()
{
  myGLCD.fillScr(0, 0, 0);
  myGLCD.setBackColor (0, 0, 0);

  myGLCD.setFont(SmallFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor (0, 0, 0);
  myGLCD.print("On", 25, 79);
  myGLCD.print("Off", 25, 165);
  myGLCD.print("Smoothing", 225, 85);

  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor (0, 0, 0);
  myGLCD.print("pH Settings - page 2", CENTER, 0);

  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor (255, 255, 255);
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect (71, 72, 143, 101);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawRoundRect (71, 72, 143, 101);
  myGLCD.setColor(0, 0, 255);
  myGLCD.printNumI(pinHighTime, 76, 79); //Time when pump works
  myGLCD.setColor(255, 255, 255);
  myGLCD.fillRoundRect (71, 155, 143, 184);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawRoundRect (71, 155, 143, 184);
  myGLCD.setColor(0, 0, 255);
  myGLCD.printNumI(pinLowTime, 76, 162); //Time when pump stops

  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 255);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (155, 47, 182, 75);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("+", 160, 53);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (155, 96, 182, 122);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("-", 160, 102);

  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (155, 131, 182, 159);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("+", 160, 137);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (155, 180, 182, 206);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("-", 160, 186);
  myGLCD.setColor(255, 255, 0);
  myGLCD.drawLine(167, 78, 167, 95);
  myGLCD.drawLine(168, 78, 168, 95);
  myGLCD.drawLine(167, 162, 167, 179);
  myGLCD.drawLine(168, 162, 168, 179);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (198, 100, 315, 130);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("ON  OFF", 200, 107);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (10, 218, 29, 235);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("<", 13, 220);
}

void InitDHT()
{
  pinMode(dht_dpin,OUTPUT);
  digitalWrite(dht_dpin,HIGH);
}

void ReadDHT()
{
  bGlobalErr=0;
  byte dht_in;
  byte i;
  digitalWrite(dht_dpin,LOW);
  delay(23);
  digitalWrite(dht_dpin,HIGH);
  delayMicroseconds(40);
  pinMode(dht_dpin,INPUT);
  dht_in=digitalRead(dht_dpin);

  if(dht_in)
  {
    bGlobalErr=1;//dht start condition 1 not met
    return;
  }

  delayMicroseconds(80);
  dht_in=digitalRead(dht_dpin);

  if(!dht_in)
  {
    bGlobalErr=2;//dht start condition 2 not met
    return;
  }

  delayMicroseconds(80);
  for (i=0; i<5; i++)
    dht_dat[i] = read_dht_dat();

  pinMode(dht_dpin,OUTPUT);

  digitalWrite(dht_dpin,HIGH);

  byte dht_check_sum =
    dht_dat[0]+dht_dat[1]+dht_dat[2]+dht_dat[3];

  if(dht_dat[4]!= dht_check_sum)
  {
    bGlobalErr=3;
  }
};


byte read_dht_dat()
{
  byte i = 0;
  byte result=0;
  for(i=0; i< 8; i++)
  {
    while(digitalRead(dht_dpin)==LOW);
    delayMicroseconds(45);

    if (digitalRead(dht_dpin)==HIGH)
      result |=(1<<(7-i));

    while (digitalRead(dht_dpin)==HIGH);
  }
  return result;
}


void fotoLoop()
{
  lightADCReading = analogRead(lightSensor);
  // Calculating the voltage of the ADC for light
  lightInputVoltage = 5.0 * ((double)lightADCReading / 1024.0);
  // Calculating the resistance of the photoresistor in the voltage divider
  lightResistance = (10.0 * 5.0) / lightInputVoltage - 10.0;
  // Calculating the intensity of light in lux       
  currentLightInLux = 255.84 * pow(lightResistance, -10/9);
}


void phIncreaseSetpoint()
{
  Setpoint = Setpoint + 0.01;
  if (Setpoint >= 9.00)
  {
    Setpoint = 9.00;
  }
  if (page == 2)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.printNumF(Setpoint, 2, 76, 79);
  }
}


void phDecreaseSetpoint()
{
  Setpoint = Setpoint - 0.01;
  if (Setpoint <= 3.00)
  {
    Setpoint = 3.00;
  }
  if (page == 2)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.printNumF(Setpoint, 2, 76, 79);
  }
}


void phIncreaseHysteris()
{
  SetHysteris = SetHysteris + 0.01;
  if (SetHysteris >= 9.00)
  {
    SetHysteris = 9.00;
  }
  if (page == 2)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.printNumF(SetHysteris, 2, 76, 162);
  }
}


void phDecreaseHysteris()
{
  SetHysteris = SetHysteris - 0.01;
  if (SetHysteris <= 0.01)
  {
    SetHysteris = 0.01;
  }
  if (page == 2)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.printNumF(SetHysteris, 2, 76, 162);
  }
}


void DecreasePumpHighTime()
{
  pinHighTime = pinHighTime - 10;
  if (pinHighTime <= 0)
  {
    pinHighTime = 0;
  }
  if (page == 4)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.printNumI(pinHighTime, 76, 79);
  }
}


void IncreasePumpHighTime()
{
  pinHighTime = pinHighTime + 10;
  if (pinHighTime >= 2000)
  {
    pinHighTime = 2000;
  }
  if (page == 4)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.printNumI(pinHighTime, 76, 79);
  }
}


void DecreasePumpLowTime()
{
  pinLowTime = pinLowTime - 100;
  if (pinLowTime <= 0)
  {
    pinLowTime = 0;
  }
  if (page == 4)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.printNumI(pinLowTime, 76, 162);
  }
}


void IncreasePumpLowTime()
{
  pinLowTime = pinLowTime + 100;
  if (pinLowTime >= 20000)
  {
    pinLowTime = 20000;
  }
  if (page == 4)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.printNumI(pinLowTime, 76, 162);
  }
}


void FanDecreaseTemp()
{
  FanTemp = FanTemp - 1;
  if (FanTemp <= 0)
  {
    FanTemp = 0;
  }
  if (page == 1)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.printNumI(FanTemp, 76, 79);
  }
}


void FanIncreaseTemp()
{
  FanTemp = FanTemp + 1;
  if (FanTemp >= 50)
  {
    FanTemp = 50;
  }
  if (page == 1)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.printNumI(FanTemp, 76, 79);
  }
}        


void FanIncreaseHumid()
{
  FanHumid = FanHumid + 1;
  if (FanHumid >= 100)
  {
    FanHumid = 100;
  }
  if (page == 1)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.printNumI(FanHumid, 76, 162);
  }
}


void FanDecreaseHumid()
{
  FanHumid = FanHumid - 1;
  if (FanHumid <= 0)
  {
    FanHumid = 0;
  }
  if (page == 1)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.printNumI(FanHumid, 76, 162);
  }
}


void FanControl()
{
  if ((dht_dat[0] >= FanHumid + fanHysteris) && (dht_dat[2] >= FanTemp + fanHysteris) || (dht_dat[0] >= FanHumid + fanHysteris + 15) || (dht_dat[2] >= FanTemp + fanHysteris + 5))
  {
    digitalWrite(ventilatorPin, HIGH);
  }
  else if ((dht_dat[0] <= FanHumid - fanHysteris) && (dht_dat[2] <= FanTemp - fanHysteris) || (dht_dat[0] <= FanHumid - fanHysteris - 10) || (dht_dat[2] <= FanTemp - fanHysteris - 5))
  {
    digitalWrite(ventilatorPin, LOW);
  }
}



void TankProgControl()
{
  if (tankProgState == 0)
  {
  }
  if (tankProgState == 1)
  {
    int levelHigh = LOW;
    int levelLow = LOW;

    levelHigh = digitalRead(floatHighPin);
    levelLow = digitalRead(floatLowPin);

    if (levelHigh == LOW)
    {
      if (page == 0)
      {
        myGLCD.setColor(0, 0, 255);
        myGLCD.print("HalfFull", 91, 207);
      }
      if (levelLow == LOW)
      {
        if (page == 0)
        {
          myGLCD.setColor(0, 0, 255);
          myGLCD.print("Filling ", 91, 207);
        }
        digitalWrite(solenoidPin, HIGH); //solenoid valve open.
      }
    }
    else
    {
      if (page == 0)
      {
        myGLCD.setColor(0, 0, 255);
        myGLCD.print("Full    ", 91, 207);
      }
      if (levelLow == HIGH)
      {
        digitalWrite(solenoidPin, LOW); //solenoid valve closed.
        if (page == 3)
        {
          myGLCD.setColor(0, 0, 255);
          myGLCD.print("OFF", 260, 171);
        }
      }
    }
  }
}


void ManualRefilProg()
{
  digitalWrite(solenoidPin, HIGH);
  if (page == 3)
  {
    myGLCD.setColor(255, 255, 0);
    myGLCD.print("ON ", 260, 171);
  }
}

void SDSetup()
{
  pinMode(53, OUTPUT);

  if (!SD.begin(chipSelect))
  {
    return;
  }
}




void SDLoop()
{
  unsigned long sdCurrentMillis = millis();
  if (sdCurrentMillis - sdPreviousMillis > sdTime)
  {
    sdPreviousMillis = sdCurrentMillis;
    if (sdState == LOW)
    {
      sdState = HIGH;              
      File dataFile = SD.open("datalog.csv", FILE_WRITE);

      if (dataFile)
      {
        now = RTC.now();
        dataFile.print(now.day(), DEC);   
        dataFile.print('/');
        dataFile.print(now.month(), DEC);
        dataFile.print('/');
        dataFile.print(now.year(), DEC);
        dataFile.print(' ');
        dataFile.print(now.hour(), DEC);
        dataFile.print(':');
        dataFile.print(now.minute(), DEC);
        dataFile.print(':');
        dataFile.print(now.second(), DEC);
        dataFile.print(", ");
        dataFile.print(pH);
        dataFile.print(", ");
        dataFile.print(dht_dat[2], DEC);
        dataFile.print(", ");
        dataFile.print(dht_dat[0], DEC);
        dataFile.print(", ");
        dataFile.print(currentLightInLux);
        dataFile.print(", ");
        dataFile.print(pmem);


        dataFile.println();
        dataFile.close();
      }
    }
    else
    {
      sdState = LOW;
    }
  }
}

void timeSetup()
{
  Wire.begin();
  RTC.begin();
}



