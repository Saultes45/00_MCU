/* ========================================
*
* Copyright Nathanael Esnault, 2022
* All Rights Reserved
* UNPUBLISHED, LICENSED SOFTWARE.
*
* Metadata
* Written by    : Nathanael Esnault (Saultes45)
* Verified by   : N/A
* Creation date : 2022-04-04 
* Version       : 0.1 (finished on ...)
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
*
*
* TODO
*
*
* Terminology
* 
*
*/



// -------------------------- Includes [0] --------------------------
//#include <Arduino.h>
// for all the sequencing
#include "global.h"

// for pump
#include "StepperPeristaltic.h"

// for OLED
#include "displayOLED.h" 

// for debug via console
#include "ConsoleUSB.h"

// for buzzer debug
#include "signalBuzzer.h"

// for user buttons
#include "buttonISR.h"

// -------------------------- Defines and Const []--------------------------


const uint8_t nbr_rollers = 7; // This is HW, cannot be changed
const uint16_t target_nbr_rollerTurns = nbr_rollers; // choose how much luiquid is dosed








// -------------------------- Global variables [3]----------------

// boolean flags for isr commands
bool needPrime = false;
bool isPrimed = false;

bool needDose = false;

// -------------------------- ISR  [4]----------------

//******************************************************************************************
void isrBTNContinue()
{
  noInterrupts(); // stops the system from generating any other ISRs
  unsigned long interrupt_time = millis();
  // If interrupts come faster than LS_DEBOUNCE_TIME, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time[INDX_BTN_CONT] > DEBOUNCE_TIME)
  {
    isrFlag[INDX_BTN_CONT] = TRIGGERED;
    //digitalWrite(LED_BUILTIN, digitalRead(BUTTON));
  }
  last_interrupt_time[INDX_BTN_CONT] = interrupt_time;
  interrupts(); // re-enables the ISRs
}
//******************************************************************************************
void isrBTNStart()
{
  noInterrupts(); // stops the system from generating any other ISRs
  unsigned long interrupt_time = millis();
  // If interrupts come faster than LS_DEBOUNCE_TIME, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time[INDX_BTN_STRT] > DEBOUNCE_TIME)
  {
    isrFlag[INDX_BTN_STRT] = TRIGGERED;
    //digitalWrite(LED_BUILTIN, digitalRead(BUTTON));
  }
  last_interrupt_time[INDX_BTN_STRT] = interrupt_time;
  interrupts(); // re-enables the ISRs
}
//******************************************************************************************
void isrBTNAbort()
{
  noInterrupts(); // stops the system from generating any other ISRs
  unsigned long interrupt_time = millis();
  // If interrupts come faster than LS_DEBOUNCE_TIME, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time[INDX_BTN_ABRT] > DEBOUNCE_TIME)
  {
    // Stop the motor and close all the valves
    // stepper.stop() //Sets a new target position that causes the stepper to stop as quickly as possible, using the current speed and acceleration parameters.
    // disable the stepper
    
    // 2

    isrFlag[INDX_BTN_ABRT] = TRIGGERED;
    //digitalWrite(LED_BUILTIN, digitalRead(BUTTON));
  }
  last_interrupt_time[INDX_BTN_ABRT] = interrupt_time;
  interrupts(); // re-enables the ISRs
}
//******************************************************************************************
void isrBTNRepeat()
{
  noInterrupts(); // stops the system from generating any other ISRs
  unsigned long interrupt_time = millis();
  // If interrupts come faster than LS_DEBOUNCE_TIME, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time[INDX_BTN_RPT] > DEBOUNCE_TIME)
  {
    isrFlag[INDX_BTN_RPT] = TRIGGERED;
    //digitalWrite(LED_BUILTIN, digitalRead(BUTTON));
  }
  last_interrupt_time[INDX_BTN_RPT] = interrupt_time;
  interrupts(); // re-enables the ISRs
}

// -------------------------- SetUp --------------------------
void setup()
{

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
  setupStepper();


  printDebugln("End of the setup");
  printDebugln("---------------------------------"); // Indicates the end of the setup

  #ifdef USE_BUZZER
  // Set up is done
  cute.play(S_MODE3);
  #endif

  lastTime = millis(); // keep track of time

// Display some info
//full steps/inter-roller
// u-steps/inter-roller
// angular inacuracy after 1 full turn


  // ISRs
	// ----
	attachISRs(); // Do that last so you won't risk stopping the "setup()"                                                  
}


// -------------------------- Loop --------------------------
void loop()
{


  // Take care of ISR flags 1/55
  //----------------------------
  if (isrFlag[INDX_BTN_CONT]==TRIGGERED || isrFlag[INDX_BTN_STRT]==TRIGGERED || isrFlag[PIN_BTN_ABRT]==TRIGGERED || isrFlag[INDX_BTN_RPT]==TRIGGERED)
  {
    Serial.println(millis());
    if(isrFlag[INDX_BTN_CONT]==TRIGGERED)
    {
      isrFlag[INDX_BTN_CONT]=UNTRIGGERED;// Reset the flag immediatly
      #ifdef USE_BUZZER
      cute.play(S_CONNECTION  ); // button pressed (Start-repeat-Continue)
      #endif
      Serial.println("Continue button pressed");
    }
    if(isrFlag[INDX_BTN_STRT]==TRIGGERED)
    {
      isrFlag[INDX_BTN_STRT]=UNTRIGGERED;// Reset the flag immediatly
      #ifdef USE_BUZZER
      cute.play(S_CONNECTION  ); // button pressed (Start-repeat-Continue)
      #endif
      Serial.println("Start button pressed");

      needPrime = true;
      isPrimed = false;

      needDose = false;

    }
    if(isrFlag[PIN_BTN_ABRT]==TRIGGERED)
    {
      isrFlag[PIN_BTN_ABRT]=UNTRIGGERED;// Reset the flag immediatly
      #ifdef USE_BUZZER
      cute.play(S_SAD           ); // Abort button presed
      #endif
      Serial.println("Abort button pressed");
    }
    if(isrFlag[INDX_BTN_RPT]==TRIGGERED)
    {
      isrFlag[INDX_BTN_RPT]=UNTRIGGERED;// Reset the flag immediatly
      #ifdef USE_BUZZER
      cute.play(S_CONNECTION  ); // button pressed (Start-repeat-Continue)
      #endif
      Serial.println("Repeat button pressed");
    }    
  }

  // Check if a motor movement is needed (const speed) - 2/55
  //----------------------------------------------------------

  // Check if we should prepare a primer or dose - 3/55
  //-----------------------------------------------------
  if (needPrime) // trumps a dose command
  {
    dose =false;

  }


} // END OF LOOP


//******************************************************************************************
void pinSetUp(void) {

  //ISR buttons
  pinMode(PIN_BTN_CONT, INPUT_PULLUP);
  pinMode(PIN_BTN_STRT, INPUT_PULLUP);
  pinMode(PIN_BTN_ABRT, INPUT_PULLUP);
  pinMode(PIN_BTN_RPT , INPUT_PULLUP);

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
void      attachISRs              (void)
{
  // initialize the pushbuttons pin as an input
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_CONT),
                  isrBTNContinue,
                  FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_STRT),
                  isrBTNStart,
                  FALLING); 
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_ABRT),
                  isrBTNAbort,
                  FALLING); 
  attachInterrupt(digitalPinToInterrupt(PIN_BTN_RPT),
                  isrBTNRepeat,
                  FALLING); 
} // END OF THE FUNCTION
//******************************************************************************************
void      primePump               (void)
{

} // END OF THE FUNCTION
//******************************************************************************************
void      dosePump                (void)
{

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
  printDebugln("Welcome to the dosing test");

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
  display.println("Dose test");

  display.setCursor(5, 40);
  // Display static text
  display.println("Version");

  display.setCursor(90, 40);
  // Display static text
  display.println(codeVersion);


  display.display();

} // END OF THE FUNCTION

//END OF FILE