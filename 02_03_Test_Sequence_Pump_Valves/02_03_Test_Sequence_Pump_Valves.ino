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

//I2C: MUX / OLED / AMS sperctrometer (not used)
// No need for inclusing "Wire.h" for OLED nor MUX

// for OLED
#include "displayOLED.h" 

// for MUX
#include "muxValves.h"

// for debug via console
#include "ConsoleUSB.h"

// for buzzer debug
#include "signalBuzzer.h"

// for colorimeter
#include "taosColorimeter.h"

// -------------------------- Defines and Const []--------------------------

// Sequence
//----------

#define NBR_FUNCTIONS       9 // Also defines how many valves sequences
#define NBR_FUNCTIONS_OP    6 // Limits the loop to the operational functions
#define NBR_FUNCTIONS_MAINT 3 // Limits the loop to the maintenence functions

// Stepper
//--------

// in StepperPeristaltic.h now
const uint16_t target_nbr_revolutions[NBR_FUNCTIONS] = {
  80, // F1 - Clean loop
  70, // F2 - Clean A side
  02, // F3 - Dose R1
  02, // F4 - Dose R2
  70, // F5 - Mix
  00, // F6 - Measure
  20, // F7 - Clean Air
  20, // F8 - Clean R1
  20, // F9 - Clean R2
};

// Valves
//--------

/* Last sequence is present is google sheets:
* ANTS Valve MUX - Inv+New - Last good one: 2022-04-03
* https://docs.google.com/spreadsheets/d/1jg5QQMtV5aC1dkNfdmuq53zsLn5vYC0wE9LcEtVTIi8/edit#gid=686154229
*/

//                                             F1 - F2 - F3 - F4 - F5 - F6 - F7- F8- F9
// const uint8_t valveMuxArray[NBR_FUNCTIONS] = {0x42,0xD2,0xA4,0xA8,0x90,0x00};
//const uint8_t valveMuxArray[NBR_FUNCTIONS] = {0x82,0x62,0x64,0x68,0x50,0x00};
const uint8_t valveMuxArray[NBR_FUNCTIONS] = {
  0x62, // F1 - Clean loop
  0x82, // F2 - Clean A side
  0x64, // F3 - Dose R1
  0x68, // F4 - Dose R2
  0x50, // F5 - Mix
  0x00, // F6 - Measure
  0x81, // F7 - Clean Air
  0x84, // F8 - Clean R1
  0x88, // F9 - Clean R2
};

// Keep track of the sequence
uint8_t cnt_functions = 0;


// Buzzer
//-------
// in "signalBuzzer.h"


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
  #endif

  setupConsole();

  //  printDebug("Initialisation...");
  //  printDebugln("done");

  setupOLED();
  infoDisplayOLED();
  delay(5000); // give us the time to read
  setupValves();
  setupStepper();

  nextFunction = true; // to start the function counter

  printDebugln("End of the setup");
  printDebugln("---------------------------------"); // Indicates the end of the setup

  #ifdef USE_BUZZER
  // Set up is done
  cute.play(S_MODE3);
  #endif

  lastTime = millis(); // keep track of time

}


// -------------------------- Loop --------------------------
void loop() {


  // Check if a stepper movement is needed (must be as fast as possible)
  if (needPump) {
    
    stepper.run(); // non blocking

    if (stepper.distanceToGo() == 0) { // check if the pump movement is finished
      nextFunction  = true; // go to the next function
      needPump      = false; // Dno not do any pump movement

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
  } // if needPump


  // Check if we have finished the previous function before we can execute the next
  //--------------------------------------------------------------------------------
  if (nextFunction) 
  {

    currentTime = millis();
    elapsedTime = currentTime - lastTime;
    printDebug("Last function took: ");
    printDebug(elapsedTime);
    printDebugln("ms to execute");
    lastTime = currentTime;

    nextFunction = false; //reset the flag

    printChangeStepOLED();

    // //Debug wait
    // delay(2000);

    // With ternary operator
    // restart at 1, NOT 0!!!!
    // (cnt_functions >= NBR_FUNCTIONS) ? cnt_functions = 1: cnt_functions++; // all
    (cnt_functions >= NBR_FUNCTIONS_OP) ? cnt_functions = 1: cnt_functions++; //OP
    // (cnt_functions >= NBR_FUNCTIONS_MAINT) ? cnt_functions = 7: cnt_functions++; // Maintenece

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
        // F1-Clean loop
        functionCleanLoop();
        break;
      case 2:
        // F2-Clean A side
        functionCleanASide();
        break;
      case 3:
        // F3-Dose R1
        functionDoseR1();
        break;
      case 4:
        // F4-Dose R2
        functionDoseR2();
        break;
      case 5:
        // F5-Mix
        functionMix();
        break;
      case 6:
        // F6-Measure
        functionMeasure();
        break;
      case 7:
        // F7-Clean Air
        functionCleanAir();
        break;
      case 8:
        // F8-Clear R1
        functionCleanR1();
        break;
      case 9:
        // F9-Clean R2
        functionCleanR2();
        break;      
      default: // code to be executed if "nextFunction" doesn't match any cases
        break;
    } //switch (cnt_functions)
  } // nextFunction

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


} // END OF THE FUNCTION

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

  printDebugln("---------------------------------");

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

  // "setup" function starts
  cute.play(S_OHOOH2);

} // END OF THE FUNCTION

//******************************************************************************************
void setupConsole(void) {

  #ifdef SERIAL_DEBUG
  Serial.begin(SERIAL_BAUDS);
  while (!Serial) // Wait here until the connection with the computer has been established
  #endif

  delay(1000);

  printDebugln("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
  printDebugln("Starting a new session");
  printDebugln("Welcome to the Stepper+Valve test");

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

  // Display welcome text
  //------------------------------
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(5, 20);
  // Display static text
  display.println("Welcome");
  display.display();

} // END OF THE FUNCTION

//******************************************************************************************
void infoDisplayOLED(void) {
  
  // Clear the buffer
  display.clearDisplay();


  // Display info
  //--------------
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(5, 15);
  // Display static text
  display.println("Valve test");

  display.setCursor(5, 40);
  // Display static text
  display.println("Version");

  display.setCursor(90, 40);
  // Display static text
  display.println(codeVersion);


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
  display.setCursor(40, 30);
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
  display.setCursor(10, 45);
  // Display static text
  display.print("function");


  display.display();

} // END OF THE FUNCTION

//******************************************************************************************
void functionCleanASide(void) {

  // Flags set up - 1/4
  //---------------------
  needPump      = true; // accel + max speed 
  nextFunction  = false; // set after the pump movement

  // Set up the motor for a mix (accel + fast) - 2/4
  //-------------------------------------------------
  printDebug("Setting target stepper acceleration to ");
  printDebug(mixAccel * uStepFactor);
  printDebug(" [micro-steps/s^2] ...");
  stepper.setAcceleration(mixAccel * uStepFactor);
  printDebugln("done");

  
  //displayStepperSettings(); // check the settings

  // Enable the motor - 3/4
  //------------------------
  printDebug("Enabling the motor...");
  digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE); // remove for more precision
  printDebugln("done");

  // Wait for the locking in place
  delay(500);

  printDebug("Number of rotations to achieve: ");
  printDebugln(target_nbr_revolutions[cnt_functions-1]);


  // Give the stepper a target position - 4/4
  //------------------------------------------
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
void functionCleanLoop(void) {

  // Flags set up - 1/4
  //---------------------
  needPump      = true; // accel + max speed 
  nextFunction  = false; // set after the pump movement

  // Set up the motor for a mix (accel + fast) - 2/4
  //-------------------------------------------------
  printDebug("Setting target stepper acceleration to ");
  printDebug(mixAccel * uStepFactor);
  printDebug(" [micro-steps/s^2] ...");
  stepper.setAcceleration(mixAccel * uStepFactor);
  printDebugln("done");

  
  //displayStepperSettings(); // check the settings

  // Enable the motor - 3/4
  //------------------------
  printDebug("Enabling the motor...");
  digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE); // remove for more precision
  printDebugln("done");

  // Wait for the locking in place
  delay(500);

  printDebug("Number of rotations to achieve: ");
  printDebugln(target_nbr_revolutions[cnt_functions-1]);


  // Give the stepper a target position - 4/4
  //------------------------------------------
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
void functionMix(void) {

  // Flags set up - 1/4
  //---------------------
  needPump      = true; // accel + max speed 
  nextFunction  = false; // set after the pump movement

  // Set up the motor for a mix (accel + fast) - 2/4
  //-------------------------------------------------
  printDebug("Setting target stepper acceleration to ");
  printDebug(mixAccel * uStepFactor);
  printDebug(" [micro-steps/s^2] ...");
  stepper.setAcceleration(mixAccel * uStepFactor);
  printDebugln("done");

  
  //displayStepperSettings(); // check the settings

  // Enable the motor - 3/4
  //------------------------
  printDebug("Enabling the motor...");
  digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE); // remove for more precision
  printDebugln("done");

  // Wait for the locking in place
  delay(500);

  printDebug("Number of rotations to achieve: ");
  printDebugln(target_nbr_revolutions[cnt_functions-1]);


  // Give the stepper a target position - 4/4
  //------------------------------------------
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

  // Flags set up - 1/4
  //-------------------
  needPump      = true; // NO accel + slow const speed 
  nextFunction  = false; // set after the pump movement

  // Set up the motor for dosing (NO accel + slow) - 2/4
  //-----------------------------------------------------
  printDebug("Setting target stepper accel to "); //TODO : actually implement const speed
  printDebug(doseAccel * uStepFactor);
  printDebug(" [micro-steps/s^2] ...");
  stepper.setAcceleration(doseAccel * uStepFactor);
  printDebugln("done");
  
  //printDebug("Setting target stepper speed (no accel) to ");
  //printDebug(doseSpeed * uStepFactor);
  //printDebug(" [micro-steps/s] ...");
  //stepper.setSpeed(doseSpeed * uStepFactor);
  //printDebugln("done");

  
  //displayStepperSettings(); // check the settings

  // Enable the motor - 3/4
  //------------------------
  printDebug("Enabling the motor...");
  digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE); // remove for more precision
  printDebugln("done");

  // Wait for the locking in place
  delay(500);

  printDebug("Number of rotations to achieve: ");
  printDebugln(target_nbr_revolutions[cnt_functions-1]);


  // Give the stepper a target position - 4/4
  //-----------------------------------------
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
void functionDoseR2(void) {

  // Flags set up - 1/4
  //-------------------
  needPump      = true; // NO accel + slow const speed 
  nextFunction  = false; // set after the pump movement

  // Set up the motor for dosing (NO accel + slow) - 2/4
  //-----------------------------------------------------
  printDebug("Setting target stepper accel to "); //TODO : actually implement const speed
  printDebug(doseAccel * uStepFactor);
  printDebug(" [micro-steps/s^2] ...");
  stepper.setAcceleration(doseAccel * uStepFactor);
  printDebugln("done");
  
  //printDebug("Setting target stepper speed (no accel) to ");
  //printDebug(doseSpeed * uStepFactor);
  //printDebug(" [micro-steps/s] ...");
  //stepper.setSpeed(doseSpeed * uStepFactor);
  //printDebugln("done");

  
  //displayStepperSettings(); // check the settings

  // Enable the motor - 3/4
  //------------------------
  printDebug("Enabling the motor...");
  digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE); // remove for more precision
  printDebugln("done");

  // Wait for the locking in place
  delay(500);

  printDebug("Number of rotations to achieve: ");
  printDebugln(target_nbr_revolutions[cnt_functions-1]);


  // Give the stepper a target position - 4/4
  //-----------------------------------------
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
void functionCleanAir(void) {

  // Flags set up - 1/4
  //---------------------
  needPump      = true; // accel + max speed 
  nextFunction  = false; // set after the pump movement

  // Set up the motor for a mix (accel + fast) - 2/4
  //-------------------------------------------------
  printDebug("Setting target stepper acceleration to ");
  printDebug(mixAccel * uStepFactor);
  printDebug(" [micro-steps/s^2] ...");
  stepper.setAcceleration(mixAccel * uStepFactor);
  printDebugln("done");

  
  //displayStepperSettings(); // check the settings

  // Enable the motor - 3/4
  //------------------------
  printDebug("Enabling the motor...");
  digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE); // remove for more precision
  printDebugln("done");

  // Wait for the locking in place
  delay(500);

  printDebug("Number of rotations to achieve: ");
  printDebugln(target_nbr_revolutions[cnt_functions-1]);


  // Give the stepper a target position - 4/4
  //------------------------------------------
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
void functionCleanR1(void) {

  // Flags set up - 1/4
  //---------------------
  needPump      = true; // accel + max speed 
  nextFunction  = false; // set after the pump movement

  // Set up the motor for a mix (accel + fast) - 2/4
  //-------------------------------------------------
  printDebug("Setting target stepper acceleration to ");
  printDebug(mixAccel * uStepFactor);
  printDebug(" [micro-steps/s^2] ...");
  stepper.setAcceleration(mixAccel * uStepFactor);
  printDebugln("done");

  
  //displayStepperSettings(); // check the settings

  // Enable the motor - 3/4
  //------------------------
  printDebug("Enabling the motor...");
  digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE); // remove for more precision
  printDebugln("done");

  // Wait for the locking in place
  delay(500);

  printDebug("Number of rotations to achieve: ");
  printDebugln(target_nbr_revolutions[cnt_functions-1]);


  // Give the stepper a target position - 4/4
  //------------------------------------------
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
void functionCleanR2(void) {

  // Flags set up - 1/4
  //---------------------
  needPump      = true; // accel + max speed 
  nextFunction  = false; // set after the pump movement

  // Set up the motor for a mix (accel + fast) - 2/4
  //-------------------------------------------------
  printDebug("Setting target stepper acceleration to ");
  printDebug(mixAccel * uStepFactor);
  printDebug(" [micro-steps/s^2] ...");
  stepper.setAcceleration(mixAccel * uStepFactor);
  printDebugln("done");

  
  //displayStepperSettings(); // check the settings

  // Enable the motor - 3/4
  //------------------------
  printDebug("Enabling the motor...");
  digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE); // remove for more precision
  printDebugln("done");

  // Wait for the locking in place
  delay(500);

  printDebug("Number of rotations to achieve: ");
  printDebugln(target_nbr_revolutions[cnt_functions-1]);


  // Give the stepper a target position - 4/4
  //------------------------------------------
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
void functionMeasure(void) {

	uint8_t cnt_wait = 0;
	for (cnt_wait = 0; cnt_wait < COLORIMETER_MEASURE_WAIT_S; cnt_wait++)
	{
	  #ifdef USE_BUZZER
		cute.play(S_MODE3         ); // Ready for colorimeter test AND Set up done
	  #endif
    delay(1000);// wait by steps of 1s
	}
	
	#ifdef USE_BUZZER
		cute.play(S_OHOOH2        ); // Done for colorimeter test  AND Set up start
	#endif

	needPump = false; // 
	nextFunction = true; // call for immediate next function


} // END OF THE FUNCTION

// END OF THE FILE
