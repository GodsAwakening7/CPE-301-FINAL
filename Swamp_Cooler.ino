#include <Stepper.h>

volatile unsigned char *ddr_h  = (unsigned char *) 0x102; //stepper button Dpin 7
volatile unsigned char *port_h = (unsigned char *) 0x101; //stepper button Dpin 7
volatile unsigned char *pin_h  = (unsigned char *) 0x100; //stepper button Dpin 7

const int stepsPerRevolution = 2038;
Stepper myStepper = Stepper(stepsPerRevolution, 29, 25, 27, 23); // Define stepper motor pins

void setup() {
  Serial.begin(9600); // Initialize serial communication
  *ddr_h &= 0xEF;     // Set PH4 (D7) as input
}

void loop() {
  bool buttonState = *pin_h; // Check if D7 button is pressed
  if(buttonState == 1) {
    stepOn(); // Move stepper motor
  }
}

void stepOn() {
  // Rotate at 10 RPM (adjust speed as needed)
  myStepper.setSpeed(10);
  myStepper.step(stepsPerRevolution/4); // Rotate 1/4 revolution
  delay(1000); // Delay after rotation (adjust as needed)
}