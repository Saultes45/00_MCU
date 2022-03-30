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

// Source: RobTillaart/PCF8574 (install via the manager but scroll down to #10, look at author)
#include "PCF8574.h" // https://www.arduino.cc/reference/en/libraries/pcf8574/

// No need for inclusing "Wire.h"

#include <CuteBuzzerSounds.h> // for buzzer

// for OLED
#include <Wire.h>
#include <Adafruit_SSD1306.h>

// -------------------------- Defines and Const []--------------------------

// Stepper motor driver pins
const uint8_t PIN_MOTOR_STEP   = 11; // Digital output 5V
const uint8_t PIN_MOTOR_DIR    = 10; // Digital output 5V
const uint8_t PIN_MOTOR_ENA    = 12; // Digital output 5V, manual logic


const uint8_t PIN_MOTOR_CFG_0   = 7; // Digital output 5V - Doesnt matter
const uint8_t PIN_MOTOR_CFG_1   = 4; // Digital output 5V
const uint8_t PIN_MOTOR_CFG_2   = 5; // Digital output 5V
const uint8_t PIN_MOTOR_CFG_3   = 6; // Digital output 5V - Doesnt matter

const uint8_t uStepFactor = 4; // set up by the MCU on the stepper driver
const uint8_t pulsesPerRevolution = 200; // Dictated by the HW only

const uint16_t interpolationFactor = 256;
String driverMode                 = "StealthChop";

// Personal MAX limits
const uint16_t personalMaxAccel = 800; //in pulses/sec so theo NOT dependent on uStepFactor
const uint16_t personalMaxSpeed = 800; //in pulses/sec so theo NOT dependent on uStepFactor

// Personal target 
const uint16_t targetAccel = personalMaxAccel; //in pulses/sec so theo NOT dependent on uStepFactor
//const uint8_t targetSpeed = 100; //in pulses/sec so theo NOT dependent on uStepFactor

#define PUMP_ENABLE   LOW
#define PUMP_DISABLE  HIGH

#define MUX_ADDRSS 0x20 
#define NBR_FUNCTIONS 6 // Also defines how many valves
//                                        F1 - F2 - F3 - F4 - F5 - F6
uint8_t valveMuxArray[NBR_FUNCTIONS] = {0x042,0xD2,0xA4,0xA8,0x90,0x00};


// Valves INDX
//-----------
// Side A
uint8_t valveAir            = 0;
uint8_t valveSample         = 1;
uint8_t valveR1             = 2;
uint8_t valveR2             = 3;
uint8_t valveRecirculation  = 4;
uint8_t valveLoopDiscard    = 5; 


// Side B
uint8_t valvePumpDiscard    = 6;
uint8_t valveLoop           = 7;

// For test purposes
//---------------------

uint8_t cnt_functions = 0;

// Console
//-----------
#define SERIAL_DEBUG // Uncomment to let the board print debug statements through the debug port (USB)
#define SERIAL_BAUDS 9600

// Buzzer
//#define USE_BUZZER
#define BUZZER_PIN 8

#define USE_OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#ifdef USE_OLED
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

// -------------------------- Macros [1] --------------------------
#ifdef SERIAL_DEBUG
  #define printDebug(message)   Serial.print(message)
  #define printDebugln(message) Serial.println(message)
#else
  #define printDebug(message) // Leave the macro empty
  #define printDebugln(message) // Leave the macro empty
#endif


// -------------------------- Global variables [2]----------------
//uint16_t target_nbr_intrarollers = 1; // nbr intra-roller space to achieve (volume goal)
//const uint8_t nbr_rollers = 7; // number of rollers on the pump head (for 1 rotation)
//long target_position = (long)( round( target_nbr_intrarollers / (float)(nbr_rollers) * pulsesPerRevolution * uStepFactor ) );

uint16_t target_nbr_revolutions = 1; // nbr revolutions to achieve (position goal)

// Define a stepper and the pins it will use
AccelStepper stepper(AccelStepper::DRIVER, PIN_MOTOR_STEP, PIN_MOTOR_DIR);

// I2C 8-pin mux for valve driver
PCF8574 VALVE_MUX(MUX_ADDRSS);


// -------------------------- Function declaration [2]----------------
void      displayStepperSettings  (void);
void      pinSetUp                (void);
void      setupValves             (void);
void      setupStepper            (void);
void      setupBuzzer             (void);
void      setupOLED               (void);





// -------------------------- SetUp --------------------------
void setup()
{

  pinSetUp();
  
  #ifdef USE_BUZZER
  
    setupBuzzer();

    // Set up starts
    cute.play(S_OHOOH2);
    
  #endif

  #ifdef SERIAL_DEBUG
  Serial.begin(SERIAL_BAUDS);
  while(!Serial) // Wait here until the connection with the computer has been established
  #endif

  delay(1000);

  printDebugln("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
  printDebugln("Starting a new session");
  printDebugln("Welcome to the Stepper+Valve test");

  
//  printDebug("Initialisation...");
//  printDebugln("done");

 
setupOLED();
setupValves();
setupStepper();




  printDebugln("End of the setup");
  printDebugln("---------------------------------"); // Indicates the end of the setup

    #ifdef USE_BUZZER

    // Set up is done
    cute.play(S_MODE3);
    
  #endif
  
}


// -------------------------- Loop --------------------------
void loop()
{

// Write a value to the whole pins at once
  printDebug("counter F");
  printDebugln(cnt_functions+1);

  VALVE_MUX.write8(valveMuxArray[cnt_functions]);
  printDebugln(valveMuxArray[cnt_functions]);

if (needPump)
{
  if (stepper.distanceToGo() == 0)
  {
    // go to the next function
    nextFunction = true;
  }
}
switch (cnt_functions)
{
    case 1: 
        break;
    case 2: 
        break;
    default: // code to be executed if n doesn't match any cases
}

 
  
if (nextFunction)
{
    // With ternary operator
  (cnt_functions >= NBR_FUNCTIONS-1) ?  cnt_functions = 0 : cnt_functions++;
  delay(1000);

  if (cnt_functions == 0)
  {
    delay(2000);
    printDebugln("------------New cycle--------------");
  }
}



  // If at the end of travel go to the other end
  if (stepper.distanceToGo() == 0)
  {

    // Disable the motor (inverse logic)
    digitalWrite(PIN_MOTOR_ENA,PUMP_DISABLE);

    #ifdef USE_BUZZER
    // Set up is done
    cute.play(S_MODE3);
  #endif
    
    delay(3000);

      stepper.moveTo(-1 * target_nbr_revolutions * pulsesPerRevolution * uStepFactor);
  
  }

  stepper.run(); // non blocking

}





//******************************************************************************************
void      pinSetUp                (void)
{

//  // initialize digital pin LED_BUILTIN as an output.
//  pinMode(LED_BUILTIN, OUTPUT);
  
  // initialize digital pin PIN_MOTOR_ENA as an output.
  pinMode(PIN_MOTOR_ENA, OUTPUT);

    // Disable the motor (logic inverse)
  digitalWrite(PIN_MOTOR_ENA,PUMP_DISABLE);

  // set motor configuration
//+--------------+-----------+----------------------------+---------------------------------------+
//| Config Index | MCU or SB |            Value           |                 Effect                |
//+--------------+-----------+----------------------------+---------------------------------------+
//|     CFG_0    |    JP1    | GND                        | Chopper off time = 140                |
//+--------------+-----------+----------------------------+---------------------------------------+
//|     CFG_1    |    MCU    | HiZ (INPUT)                |                                       |
//+--------------+-----------+----------------------------+  4/256 StealthChop                    |
//|     CFG_2    |    MCU    | VCC_IO                     |                                       |
//+--------------+-----------+----------------------------+---------------------------------------+
//|     CFG_3    |    JP2    | HiZ                        | Mode of current AIN: external sense V |
//+--------------+-----------+----------------------------+---------------------------------------+
//|     CFG_4    |     SB    | GND                        | Chopper hysteresis = 5                |
//+--------------+-----------+----------------------------+---------------------------------------+
//|     CFG_5    |     SB    | VCC_IO                     | Chopper blank Time = 24               |
//+--------------+-----------+----------------------------+---------------------------------------+
//|     CFG_6    |    MCU    | Changes from VCC_IO to GND | Disable/Enable the stepper            |
//+--------------+-----------+----------------------------+---------------------------------------+
//  

  pinMode(PIN_MOTOR_CFG_1, INPUT); // for HiZ
  pinMode(PIN_MOTOR_CFG_2, OUTPUT);
  
//  digitalWrite(PIN_MOTOR_CFG_1,LOW); // 4/256 StealthChop
  digitalWrite(PIN_MOTOR_CFG_2,HIGH); // 4/256 StealthChop



} // END OF FUNCTION




//******************************************************************************************
void setupValves(void)
{

   printDebug("PCF8574_LIB_VERSION:\t");
  printDebugln(PCF8574_LIB_VERSION);

// Valves
//-------
  printDebug("Trying to connect to PCF8574 at I2C address ");
  printDebug(MUX_ADDRSS);
  printDebug("(dec)...");
  if (!VALVE_MUX.begin())
  {
    printDebugln("PCF8574 could not be initialized");
  }
  if (!VALVE_MUX.isConnected())
  {
    printDebugln("NOT connected");
  }
  else
  {
    printDebugln("connected");
  }

  
}// END OF THE FUNCTION

//******************************************************************************************
void setupStepper(void)
{

// Stepper set MAX limit values
//------------------------------


  // To calculate max acceleration, max speed must be set first
  
  printDebug("Setting max SPEED to: ");
  printDebugln(personalMaxSpeed * uStepFactor);
  stepper.setMaxSpeed(personalMaxSpeed * uStepFactor);
  
  printDebug("Setting max ACCEL to: ");
  printDebugln(personalMaxAccel * uStepFactor);
  stepper.setMaxSpeed(personalMaxAccel * uStepFactor);
  
// Stepper accel + pos target
//----------------------------
  stepper.setAcceleration(targetAccel * uStepFactor);
  // do NOT set the speed with setSpeed, it is ONLY for
  stepper.moveTo(target_nbr_revolutions * pulsesPerRevolution * uStepFactor);


// Stepper display settings
//-------------------------
  displayStepperSettings();

  // Enable the motor (logic inverse)
  digitalWrite(PIN_MOTOR_ENA,PUMP_ENABLE);
  
}// END OF THE FUNCTION



//******************************************************************************************
void displayStepperSettings(void)
{
  // Display settings to UART
  //-------------------------
  printDebugln("---------------------------------");


  printDebug("Current full steps per rotation: ");
  printDebugln(pulsesPerRevolution);
  printDebug("Current micro-steps per rotation: ");
  printDebugln(pulsesPerRevolution * uStepFactor);
  printDebug("Current micro-steping factor: ");
  printDebugln(uStepFactor);
    printDebug("Current interpolation factor: ");
  printDebugln(interpolationFactor);
    printDebug("Current mode: ");
  printDebugln(driverMode);

  Serial.println("---------------------------------");

//MAX
  printDebug("Current max speed: ");
  printDebugln(stepper.maxSpeed()); // Returns the most recent speed in steps per second

  printDebug("Current max accel: ");
  printDebugln("No function to read back MAX set acceleration");

// Current targets
  printDebug("Current target speed: ");
  printDebugln(stepper.speed()); // The most recently set speed

  printDebug("Current target accel: ");
  printDebugln("No function to read back TARGET set acceleration");

// Position target
 printDebug("Distance to go: ");
  printDebugln(stepper.distanceToGo());

   printDebug("Target position: ");
  printDebugln(stepper.targetPosition());

     printDebug("Current position: ");
  printDebugln(stepper.currentPosition());



}// END OF THE FUNCTION




//******************************************************************************************
void setupBuzzer(void)
{
  cute.init(BUZZER_PIN);
  
}// END OF THE FUNCTION

//******************************************************************************************
void setupOLED(void)
{
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    printDebugln(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();


  // Follow (2022-02-22)
  //https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/

    // Display text “Valve Test”
  //------------------------------
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(5, 20);
  // Display static text
  display.println("Valve Test");
  display.display(); 
  
}// END OF THE FUNCTION




// END OF THE FILE
