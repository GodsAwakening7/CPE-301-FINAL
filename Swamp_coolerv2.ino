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

// *** REGISTERS ***
// SERIAL TRANSMISSION 
volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;
 
// ADC
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

// Timer Pointers
volatile unsigned char *myTCCR1A  = 0x80;
volatile unsigned char *myTCCR1B  = 0x81;
volatile unsigned char *myTCCR1C  = 0x82;
volatile unsigned char *myTIMSK1  = 0x6F;
volatile unsigned char *myTIFR1   = 0x36;
volatile unsigned int  *myTCNT1   = 0x84;

// GPIO
// LEDS - 7: PH4 (RED), 8: PH5 (YELLOW), 9: PH6 (GREEN), 10: PB4 (BLUE)
// BUTTONS - 12: PB6 (STOP), 13: PB7 (RESET)
// FAN CONTROL - 31: PC6 (IN4), 33: PC4 (IN3), 35: PC2 (ON/OFF/SPEED)
// for port Hs
volatile unsigned char* port_h = (unsigned char*) 0x102;
volatile unsigned char* ddr_h = (unsigned char*) 0x101;
volatile unsigned char* pin_h = (unsigned char*) 0x100;
// for port Bs
volatile unsigned char* port_b = (unsigned char*) 0x25;
volatile unsigned char* ddr_b = (unsigned char*) 0x24;
volatile unsigned char* pin_b = (unsigned char*) 0x23;
// for port Cs
volatile unsigned char* port_c = (unsigned char*) 0x28;
volatile unsigned char* ddr_c = (unsigned char*) 0x27;
volatile unsigned char* pin_c = (unsigned char*) 0x26;

// Button flags
bool startButton;
bool resetButton;
bool stopButton;

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

void my_delay(unsigned long duration){
  unsigned long delayThreshold = duration / 4.1;
  delayCounter = 0;
  while (delayCounter < delayThreshold) {} // wait until delay finishes
}
