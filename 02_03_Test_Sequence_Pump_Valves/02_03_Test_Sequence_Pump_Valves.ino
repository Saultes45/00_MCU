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

// for pump
#include <AccelStepper.h>


// for buzzer
#include <CuteBuzzerSounds.h> 

//I2C: MUX / OLED / AMS sperctrometer (not used)
// No need for inclusing "Wire.h" for OLED nor MUX

// for OLED
#include <Wire.h>
#include <Adafruit_SSD1306.h>

// for MUX
// Source: RobTillaart/PCF8574 (install via the manager but scroll down to #10, look at author)
#include "PCF8574.h" // https://www.arduino.cc/reference/en/libraries/pcf8574/

// -------------------------- Defines and Const []--------------------------

// Sequence
//----------

#define NBR_FUNCTIONS 6 // Also defines how many valves sequences

// Stepper
//--------

// Enable logic (active low)
#define PUMP_ENABLE   LOW
#define PUMP_DISABLE  HIGH

// Movement pins
const uint8_t PIN_MOTOR_STEP   = 11; // Digital output 5V
const uint8_t PIN_MOTOR_DIR    = 10; // Digital output 5V
const uint8_t PIN_MOTOR_ENA    = 12; // Digital output 5V, manual logic

// Param
const uint8_t PIN_MOTOR_CFG_0   = 7; // Digital output 5V - Doesn't matter
const uint8_t PIN_MOTOR_CFG_1   = 4; // Digital output 5V
const uint8_t PIN_MOTOR_CFG_2   = 5; // Digital output 5V
const uint8_t PIN_MOTOR_CFG_3   = 6; // Digital output 5V - Doesn't matter

// To calculate position
const uint8_t uStepFactor = 4; // set up by the MCU on the stepper driver
const uint8_t pulsesPerRevolution = 200; // Dictated by the HW only

const uint16_t interpolationFactor = 256;
String driverMode                 = "StealthChop";

// Personal MAX limits
const uint16_t personalMaxAccel = 200; //in [pulses/sec^2] so theo NOT dependent on uStepFactor
const uint16_t personalMaxSpeed = 200; //in [pulses/sec] so theo NOT dependent on uStepFactor

// dose/clean pump parameters
// REMOVE THE NEXT LINE
const uint16_t targetAccel = personalMaxAccel; //in pulses/sec^2 so theo NOT dependent on uStepFactor
//const uint8_t targetSpeed = 100; //in pulses/sec so theo NOT dependent on uStepFactor

const uint16_t cleanSpeed = personalMaxSpeed; // Do not use, in [pulses/sec]
//const uint16_t cleanAccel = 150; // in [pulses/sec^2]

const uint16_t doseSpeed = 10; //  in [pulses/sec]
//const uint16_t doseAccel = 150; // Do not use, in [pulses/sec^2]

const uint16_t target_nbr_revolutions[NBR_FUNCTIONS] = {5,8,1,1,3,0};




// Valves
//--------
#define MUX_ADDRSS 0x20 
//                                             F1 - F2 - F3 - F4 - F5 - F6
// const uint8_t valveMuxArray[NBR_FUNCTIONS] = {0x42,0xD2,0xA4,0xA8,0x90,0x00};
const uint8_t valveMuxArray[NBR_FUNCTIONS] = {0x82,0xD2,0x64,0x68,0x50,0x00};

// Keep track of the sequence
uint8_t cnt_functions = 0;

// Valves INDX TODO: not actually used
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


// Console
//--------
#define SERIAL_DEBUG // Uncomment to let the board print debug statements through the debug port (USB)
#define SERIAL_BAUDS 115200

// Buzzer
//-------
#define USE_BUZZER
#define BUZZER_PIN 8

//OLED
//----
#define USE_OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
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


// Define a stepper and the pins it will use
AccelStepper stepper(AccelStepper::DRIVER, PIN_MOTOR_STEP, PIN_MOTOR_DIR);

// I2C 8-pin mux for valve driver
PCF8574 VALVE_MUX(MUX_ADDRSS);

// Sequencing
//------------
bool needPump = false;
bool nextFunction = false;


// -------------------------- Function declaration [8]----------------
void      displayStepperSettings  (void);
void      pinSetUp                (void);

void      setupValves             (void);
void      setupStepper            (void);
void      setupBuzzer             (void);
void      setupOLED               (void);

void      printFunctionOLED       (void);
void      printChangeStepOLED     (void);





// -------------------------- SetUp --------------------------
void setup() {

  pinSetUp();

  #ifdef USE_BUZZER
  setupBuzzer();
  // Set up starts
  cute.play(S_OHOOH2);
  #endif

  #ifdef SERIAL_DEBUG
  Serial.begin(SERIAL_BAUDS);
  while (!Serial) // Wait here until the connection with the computer has been established
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

  nextFunction = true; // to start the function counter

  printDebugln("End of the setup");
  printDebugln("---------------------------------"); // Indicates the end of the setup

  #ifdef USE_BUZZER

  // Set up is done
  cute.play(S_MODE3);

  #endif

}


// -------------------------- Loop --------------------------
void loop() {

  // Check if we have finished the previous function before we can go to the next
  //------------------------------------------------------------------------------
  if (nextFunction) 
  {
    nextFunction = false; //reset the flag

    printChangeStepOLED();

    //Debug wait
    delay(2000);

    // With ternary operator
    // (cnt_functions >= NBR_FUNCTIONS) ? cnt_functions = 1: cnt_functions++;
    (cnt_functions >= 2) ? cnt_functions = 1: cnt_functions++; // restart at 1, NOT 0!!!!

    #ifdef USE_BUZZER
      cute.play(S_BUTTON_PUSHED ); // Start of a new step
    #endif
    

    if (cnt_functions == 1) 
    { // if this is a new cycle
      delay(2000); // wait a bit

      printDebugln("------------New cycle--------------");
 
    } // new cycle

    printFunctionOLED();

    printDebug("Executing function F");
    printDebugln(cnt_functions);

    // Change the all MUX pin at once
    printDebug("Changing valves state to ");
    printDebug(valveMuxArray[cnt_functions-1]);
    printDebug("...");
    VALVE_MUX.write8(valveMuxArray[cnt_functions-1]);
    printDebugln("done");

    // Now that the valves are in the correct position, let's pump!
    switch (cnt_functions) {
      case 0:
        // You should enter here only once, when you have finished set up
        #ifdef USE_BUZZER
          cute.play(S_HAPPY_SHORT); // Auto mode start
        #endif
        

        needPump = false; // let's prime, but not yet
        nextFunction = true; // call for immediate next function

      case 1:
        // F1-Clean A side
        needPump = true; // accel + max speed 
        nextFunction = false; // set after the pump movement

        printDebug("Enabling the motor...");
        // Enable the motor (inverse logic)
        digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE); // remove for more precision
        printDebugln("done");

        // Wait for the locking in place
        delay(500);

        printDebug("Number of rotations to achieve: ");
        printDebugln(target_nbr_revolutions[cnt_functions-1]);



        printDebug("Starting the motor to ");
        printDebug(-1 * (long)target_nbr_revolutions[cnt_functions-1] * (long)pulsesPerRevolution * (long)uStepFactor);
        printDebug(" ... ");
        // Start the pump TODO: optimise
        // move() is long relative
        // moveTo() is long absolute
        stepper.move(-1 * (long)target_nbr_revolutions[cnt_functions-1] * (long)pulsesPerRevolution * (long)uStepFactor);
        printDebugln("done");

        break;

      case 2:
        // F2-Clean Loop
        needPump = true; // accel + max speed 
        nextFunction = false; // set after the pump movement

        printDebug("Enabling the motor...");
        // Enable the motor (inverse logic)
        digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE); // remove for more precision
        printDebugln("done");

        // Wait for the locking in place
        delay(500);

        printDebug("Number of rotations to achieve: ");
        printDebugln(target_nbr_revolutions[cnt_functions-1]);



        printDebug("Starting the motor to ");
        printDebug(-1 * (long)target_nbr_revolutions[cnt_functions-1] * (long)pulsesPerRevolution * (long)uStepFactor);
        printDebug(" ... ");
        // Start the pump TODO: optimise
        // move() is long relative
        // moveTo() is long absolute
        stepper.move(-1 * (long)target_nbr_revolutions[cnt_functions-1] * (long)pulsesPerRevolution * (long)uStepFactor);
        printDebugln("done");
        break;
      case 3:
        // F3-Dose R1
        needPump = false; // 
        nextFunction = true; // call for immediate next function
        break;
      case 4:
        // F4-Dose R2
        needPump = false; // 
        nextFunction = true; // call for immediate next function
        break;
      case 5:
        // F5-Mix
        needPump = false; // 
        nextFunction = true; // call for immediate next function
        break;
      case 6:
        // F6-Measure
        #ifdef USE_BUZZER
          cute.play(S_MODE3         ); // Ready for colorimeter test AND Set up done
          delay(1000);
          cute.play(S_MODE3         ); // Ready for colorimeter test AND Set up done
          delay(1000);
          cute.play(S_MODE3         ); // Ready for colorimeter test AND Set up done
          delay(1000);
          cute.play(S_OHOOH2        ); // Done for colorimeter test  AND Set up start
        #endif
       
        
        needPump = false; // 
        nextFunction = true; // call for immediate next function
        break;
      default: // code to be executed if n doesn't match any cases
        break;
      }


  } // nextFunction
  else // we keep on the same function, we might be waiting for the pump to finish
  {
    if (needPump) {

      stepper.run(); // non blocking

      if (stepper.distanceToGo() == 0) { // check if the pump movement is finished
        nextFunction = true; // go to the next function
        needPump = false;

        printDebugln("Pump movement done!");

        printDebug("Disabling the motor...");
        // Disable the motor (inverse logic)
        digitalWrite(PIN_MOTOR_ENA, PUMP_DISABLE); // remove for more precision
        printDebugln("done");

        // TODO: display how much time it took to perform the movement

        #ifdef USE_BUZZER
          // The motor stops
          cute.play(S_MODE3);
        #endif
      }
    }
  }


} // END OF LOOP





//******************************************************************************************
void pinSetUp(void) {

  //  // initialize digital pin LED_BUILTIN as an output.
  //  pinMode(LED_BUILTIN, OUTPUT);

  // initialize digital pin PIN_MOTOR_ENA as an output.
  pinMode(PIN_MOTOR_ENA, OUTPUT);

  // Disable the motor (logic inverse)
  digitalWrite(PIN_MOTOR_ENA, PUMP_DISABLE);

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
  digitalWrite(PIN_MOTOR_CFG_2, HIGH); // 4/256 StealthChop

} // END OF FUNCTION

//******************************************************************************************
void setupValves(void) {

  printDebug("PCF8574_LIB_VERSION:\t");
  printDebugln(PCF8574_LIB_VERSION);

  // Valves
  //-------
  printDebug("Trying to connect to PCF8574 at I2C address ");
  printDebug(MUX_ADDRSS);
  printDebug("(dec)...");
  if (!VALVE_MUX.begin()) {
    printDebugln("PCF8574 could not be initialized");
  }
  if (!VALVE_MUX.isConnected()) {
    printDebugln("NOT connected");
  } else {
    printDebugln("connected");
  }

} // END OF THE FUNCTION

//******************************************************************************************
void setupStepper(void) {

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
  // do NOT set the speed with "setSpeed", it is ONLY for "runSpeed()"
  // stepper.moveTo(target_nbr_revolutions * pulsesPerRevolution * uStepFactor);  // absolute
  stepper.moveTo(0); // absolute

  // Stepper display settings
  //-------------------------
  displayStepperSettings();

  // Enable the motor (logic inverse)
  digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE);

} // END OF THE FUNCTION

//******************************************************************************************
void displayStepperSettings(void) {
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

} // END OF THE FUNCTION

//******************************************************************************************
void setupBuzzer(void) {
  cute.init(BUZZER_PIN);

} // END OF THE FUNCTION

//******************************************************************************************
void setupOLED(void) {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    printDebugln(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
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

} // END OF THE FUNCTION

//******************************************************************************************
void printFunctionOLED(void) {


  // Clear the buffer
  display.clearDisplay();

  // Follow (2022-02-22)
  //https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/

  // Display text "F-"
  //------------------
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(5, 20);
  // Display static text
  display.print("F-");
  display.println(cnt_functions);
  
  display.display();

} // END OF THE FUNCTION

//******************************************************************************************
void printChangeStepOLED(void) {


  // Clear the buffer
  display.clearDisplay();

  // Follow (2022-02-22)
  //https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/

  // Display text "F-"
  //------------------
  display.setTextSize(2);
  display.setTextColor(WHITE);
  
  //line1
  display.setCursor(10, 20);
  // Display static text
  display.print("Starting a");
  
  //line 2
  display.setCursor(10, 50);
  // Display static text
  display.print("new function");


  display.display();

} // END OF THE FUNCTION


// END OF THE FILE
