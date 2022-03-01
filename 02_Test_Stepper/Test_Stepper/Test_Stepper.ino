/* ========================================
*
* Copyright University of Auckland Ltd, 2022
* All Rights Reserved
* UNPUBLISHED, LICENSED SOFTWARE.
*
* Metadata
* Written by    : Nathanaël Esnault
* Verified by   : Nathanaël Esnault
* Creation date : 2022-02-21
* Version       : 0.1 (finished on 2021-..-..)
* Modifications :
* Known bugs    :
*
*
* Possible Improvements
*
*
* Notes
*
*
* Ressources (Boards + Libraries Manager)
* Arduino Mega 2556
*
* TODO
*
*
* PUMP = stepper = pump = peristaltic 
* ========================================
*/

// SOURCE:
// Make a single stepper bounce from one limit to another
//
// Copyright (C) 2012 Mike McCauley
// $Id: Random.pde,v 1.1 2011/01/05 01:51:01 mikem Exp mikem $



// -------------------------- Includes --------------------------
#include <AccelStepper.h>


// -------------------------- Defines and Const []--------------------------

// Stepper motor driver pins
const uint8_t PIN_MOTOR_STEP   = 11; // Digital output 5V
const uint8_t PIN_MOTOR_DIR    = 10; // Digital output 5V
const uint8_t PIN_MOTOR_ENA    = 12; // Digital output 5V, manual logic


const uint8_t stepFactor = 1;
const uint8_t pulsesPerRevolution = 200;


#define PUMP_ENABLE   LOW
#define PUMP_DISABLE  HIGH


// Console
//-----------
#define SERIAL_DEBUG // Uncomment to let the board print debug statements through the debug port (USB)
#define SERIAL_BAUDS 9600

// -------------------------- Macros [1] --------------------------
#ifdef SERIAL_DEBUG
  #define printDebug(message)   Serial.print(message)
  #define printDebugln(message) Serial.println(message)
#else
  #define printDebug(message) // Leave the macro empty
  #define printDebugln(message) // Leave the macro empty
#endif


// -------------------------- Global variables [2]----------------
uint16_t nbr_revolutions = 2;

// Define a stepper and the pins it will use
AccelStepper stepper(AccelStepper::DRIVER, PIN_MOTOR_STEP, PIN_MOTOR_DIR);



void      displayStepperSettings  (void);



// -------------------------- SetUp --------------------------
void setup()
{

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  // initialize digital pin PIN_MOTOR_ENA as an output.
  pinMode(PIN_MOTOR_ENA, OUTPUT);

  // Enable the motor (logic inverse)
  digitalWrite(PIN_MOTOR_ENA,PUMP_ENABLE);

  #ifdef SERIAL_DEBUG
  Serial.begin(SERIAL_BAUDS);
  while(!Serial) // Wait here until the connection with the computer has been established
  #endif

  delay(1000);

  printDebugln("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
  printDebugln("Starting a new session");
  printDebugln("Welcome to the stepper motor test");
  


  //printDebug("Initialisation...");


  printDebugln("Setting max speed to :");
  stepper.setMaxSpeed(10000 * stepFactor);
  printDebugln("Setting max accel to :");
  stepper.setAcceleration(100000 * stepFactor);
  stepper.moveTo(nbr_revolutions * pulsesPerRevolution * stepFactor);

  displayStepperSettings();

  printDebugln("---------------------------------"); // Indicates the end of the setup
  
}


// -------------------------- Loop --------------------------
void loop()
{

//  while ( (stepper.distanceToGo() != 0) )
//  {
//    //stepper.runSpeed();
//    stepper.run();
//  }
//  
  // If at the end of travel go to the other end
  if (stepper.distanceToGo() == 0)
  {
    delay(3000);
    stepper.moveTo(-stepper.currentPosition());
  }

  stepper.run();
}




//******************************************************************************************
void displayStepperSettings(void)
{
  // Display settings to UART
  //-------------------------
  Serial.println("---------------------------------");

  Serial.print("Current micro-steps settings: ");
//  Serial.println(microSteppingFactorList[indx_microSteppingFactorList]);
  Serial.println("");

  Serial.print("Current micro-steps per rotation: ");
//  Serial.println(microstepsPerRevolution);
  Serial.println("");

  Serial.println("---------------------------------");

  // To calculate max acceleration, max speed must be set first

  Serial.print("Current max speed: ");
  Serial.println(stepper.maxSpeed ()); // Returns the most recent speed in steps per second
//  Serial.print("Setting max speed to: ");
//  Serial.println(max_allowedMicroStepsPerSeconds); // Returns the most recent speed in [steps per second]
//  stepper.setMaxSpeed(max_allowedMicroStepsPerSeconds);
//  Serial.print("New max speed is: ");
//  Serial.println(stepper.maxSpeed ()); // Returns the most recent speed in steps per second (float)

//  Serial.print("Setting acceleration to: ");
//  Serial.println(max_allowedMicroStepsPerSecondsPerSeconds);
//  stepper.setAcceleration(max_allowedMicroStepsPerSecondsPerSeconds);
  // No function to read back set acceleration
  // NOTICE: Calling setAcceleration() is expensive, since it requires a square root to be calculated.

}// END OF THE FUNCTION



// END OF THE FILE
