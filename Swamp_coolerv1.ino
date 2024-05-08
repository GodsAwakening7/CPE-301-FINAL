#include <DHT.h>
#include <DHT_U.h>

// *** MACROS ***
#define RDA 0x80
#define TBE 0x20
// for pin manipulation!
#define WRITE_HIGH(address, pin_num)  address |= (0x01 << pin_num);
#define WRITE_LOW(address, pin_num)  address &= ~(0x01 << pin_num);
#define PIN_READ(address, pin_num) (address & (1 << pin_num)) != 0;

// *** INCLUDES ***
// LCD DISPLAY
#include <LiquidCrystal.h>
const int RS = 22, EN = 24, D4 = 3, D5 = 4, D6 = 5, D7 = 6;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
bool displayData;

// STEPPER
#include <Stepper.h>
const int stepsPerRevolution = 2038;
Stepper myStepper = Stepper(stepsPerRevolution, 39, 41, 43, 45); // 1N1 = 39, 1N4 = 45

// RTC
#include <Wire.h>
#include <RTClib.h>
RTC_DS1307 rtc;
DateTime now;

String date_time_to_str(DateTime obj){
  String output = String(obj.year()) + "-" + 
                  String(obj.month()) + "-" + 
                  String(obj.day()) + " " + 
                  String(obj.hour()) + ":" + 
                  String(obj.minute()) + ":" + 
                  String(obj.second());
  return output;
}

// ISR
const int startButtonPin = 2;
const int interruptNumber = digitalPinToInterrupt(startButtonPin);

volatile unsigned long overflowCounter = 0; // Counter for the overflows
volatile unsigned long delayCounter = 0; // Counter for main loop delay
volatile unsigned long buttonCounter = 0;
const unsigned long overflowsPerMinute = 60000 / 4.1; // Calculate overflows in one minute
const unsigned long overflowsPer500ms = 500 / 4.1;
bool readData = true;
bool readButtons = true;
ISR(TIMER1_OVF_vect){
  overflowCounter++;
  delayCounter++;
  buttonCounter++;
  if (overflowCounter >= overflowsPerMinute){
    readData = true;
    overflowCounter = 0;
  }
  if (buttonCounter >= overflowsPer500ms){
    readButtons = true;
    buttonCounter = true;
  }
}