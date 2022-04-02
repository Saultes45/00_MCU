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



// -------------------------- Includes --------------------------

// for all the sequencing
#include "global.h"

// for pump
#include "StepperPeristaltic.h" // personal

// for buzzer
#include <CuteBuzzerSounds.h> 

//I2C: MUX / OLED / AMS sperctrometer (not used)
// No need for inclusing "Wire.h" for OLED nor MUX

// for OLED
#include "displayOLED.h" 

// for MUX
#include "muxValves.h"

// for debug via console
#include "ConsoleUSB.h"

// -------------------------- Defines and Const []--------------------------

// Sequence
//----------

#define NBR_FUNCTIONS 6 // Also defines how many valves sequences


// RGB LED for colorimeter
//-------------------------

//#define USE_RGB_LED
const uint8_t LED_RED_PIN   = 46; // Digital output 5V - PWM capable
const uint8_t LED_GRN_PIN   = 44; // Digital output 5V - PWM capable
const uint8_t LED_BLU_PIN   = 45; // Digital output 5V - PWM capable

// Stepper
//--------

// in StepperPeristaltic.h now
const uint16_t target_nbr_revolutions[NBR_FUNCTIONS] = {60,80,1,1,3,0};

// Valves
//--------

//                                             F1 - F2 - F3 - F4 - F5 - F6
// const uint8_t valveMuxArray[NBR_FUNCTIONS] = {0x42,0xD2,0xA4,0xA8,0x90,0x00};
const uint8_t valveMuxArray[NBR_FUNCTIONS] = {0x82,0x62,0x64,0x68,0x50,0x00};

// Keep track of the sequence
uint8_t cnt_functions = 0;


// Buzzer
//-------
//#define USE_BUZZER
#define BUZZER_PIN 8

//OLED
//----
// in "displayOLED.h" now


// -------------------------- Function declaration [8] ----------------
// in global.h now



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

  printDebugln(mixAccel);

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

        // Flags set up - 1/55
        //---------------------
        needPump      = true; // accel + max speed 
        nextFunction  = false; // set after the pump movement

        // Set up the motor for a clean (accel + fast) - 2/55
        //---------------------------------------------------
        printDebug("Setting target stepper acceleration to ");
        printDebug(cleanAccel * uStepFactor);
        printDebug(" [full steps/s^2] ..."); // TODO: check, probably u-steps
        stepper.setAcceleration(cleanAccel * uStepFactor);
        printDebugln("done");

        
        //displayStepperSettings(); // check the settings

        // Enable the motor - 3/55
        //------------------------
        printDebug("Enabling the motor...");
        digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE); // remove for more precision
        printDebugln("done");

        // Wait for the locking in place
        delay(500);

        printDebug("Number of rotations to achieve: ");
        printDebugln(target_nbr_revolutions[cnt_functions-1]);


        // Give the stepper a target position - 4/55
        //-------------------------------------------
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
        functionMix();
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

  // RGB LED colorimeter
  #ifdef USE_RGB_LED
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GRN_PIN, OUTPUT);
  pinMode(LED_BLU_PIN, OUTPUT);

  analogWrite(LED_RED_PIN, 250); // 127 = 50%DC
  analogWrite(LED_GRN_PIN, 250); // 127 = 50%DC
  analogWrite(LED_BLU_PIN, 250); // 127 = 50%DC
  #endif


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
  display.setCursor(10, 10);
  // Display static text
  display.print("Starting");

  //line2
  display.setCursor(10, 25);
  // Display static text
  display.print("a new");
  
  //line 2
  display.setCursor(10, 50);
  // Display static text
  display.print("function");


  display.display();

} // END OF THE FUNCTION

//******************************************************************************************
void functionMix(void) {

  // Flags set up - 1/55
  //---------------------
  needPump      = true; // accel + max speed 
  nextFunction  = false; // set after the pump movement

  // Set up the motor for a mix (accel + fast) - 2/55
  //--------------------------------------------------------
  printDebug("Setting target stepper acceleration to ");
  printDebug(mixAccel * uStepFactor);
  printDebug(" [full steps/s^2] ..."); // TODO: check, probably u-steps
  stepper.setAcceleration(mixAccel * uStepFactor);
  printDebugln("done");

  
  //displayStepperSettings(); // check the settings

  // Enable the motor - 3/55
  //------------------------
  printDebug("Enabling the motor...");
  digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE); // remove for more precision
  printDebugln("done");

  // Wait for the locking in place
  delay(500);

  printDebug("Number of rotations to achieve: ");
  printDebugln(target_nbr_revolutions[cnt_functions-1]);


  // Give the stepper a target position - 4/55
  //-------------------------------------------
  printDebug("Starting the motor to ");
  printDebug(-1 * (long)target_nbr_revolutions[cnt_functions-1] * (long)pulsesPerRevolution * (long)uStepFactor);
  printDebug(" ... ");
  // Start the pump TODO: optimise
  // move() is long relative
  // moveTo() is long absolute
  stepper.move(-1 * (long)target_nbr_revolutions[cnt_functions-1] * (long)pulsesPerRevolution * (long)uStepFactor);
  printDebugln("done");


} // END OF THE FUNCTION



//******************************************************************************************
void functionDoseR1(void) {

  // Flags set up - 1/55
  //---------------------
  needPump      = true; // accel + max speed 
  nextFunction  = false; // set after the pump movement

  // Set up the motor for dosing (NO accel + slow) - 2/55
  //--------------------------------------------------------
  printDebug("Setting target stepper speed to ");
  printDebug(mixAccel * uStepFactor);
  printDebug(" [full steps/s] ..."); // TODO: check, probably u-steps
  stepper.setAcceleration(mixAccel * uStepFactor);
  printDebugln("done");

  
  //displayStepperSettings(); // check the settings

  // Enable the motor - 3/55
  //------------------------
  printDebug("Enabling the motor...");
  digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE); // remove for more precision
  printDebugln("done");

  // Wait for the locking in place
  delay(500);

  printDebug("Number of rotations to achieve: ");
  printDebugln(target_nbr_revolutions[cnt_functions-1]);


  // Give the stepper a target position - 4/55
  //-------------------------------------------
  printDebug("Starting the motor to ");
  printDebug(-1 * (long)target_nbr_revolutions[cnt_functions-1] * (long)pulsesPerRevolution * (long)uStepFactor);
  printDebug(" ... ");
  // Start the pump TODO: optimise
  // move() is long relative
  // moveTo() is long absolute
  stepper.move(-1 * (long)target_nbr_revolutions[cnt_functions-1] * (long)pulsesPerRevolution * (long)uStepFactor);
  printDebugln("done");


} // END OF THE FUNCTION


// END OF THE FILE
