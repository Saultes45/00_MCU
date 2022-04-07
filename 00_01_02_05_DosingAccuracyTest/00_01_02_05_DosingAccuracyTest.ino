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


// for colorimeter
#include "taosColorimeter.h"


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
//const uint16_t target_nbr_rollerTurns = nbr_rollers; // choose how much luiquid is dosed

#define DISABLE_PUMP_AFTER_RUN // uncomment to 1- enable the pump before a movement and 2- disable the pump after

const long stepperDirection = 1;// to invert direction if needed, -1 and +1 allowed only

const float prime_target_nbr_revolutions = 25.0 / (float)(nbr_rollers);

const float dose_target_nbr_revolutions = 14.0 / (float)(nbr_rollers);


// -------------------------- Global variables [3]----------------

// boolean flags for isr commands
// PRIME
bool needPrime = false;
bool isPrimed = false;
bool primeInProgress = false;

// DOSE
bool needDose = false;
bool doseInProgress = false;

// ANY
bool needPump = false; // for any pump movement (dose or prime)

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
    // Stop the motor and close all the valves (ASAP, no waiting)
    // stepper.stop() //Sets a new target position that causes the stepper to stop as quickly as possible, using the current speed and acceleration parameters.
    
    // disable the stepper (ASAP, no waiting)
    disableStepper(); // no braking, free-wheeling 
    
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
  //delay(5000); // give us the time to read
  setupStepper();


  printDebugln("End of the setup");
  printDebugln("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"); // Indicates the end of the setup

  #ifdef USE_BUZZER
  // Set up is done
  cute.play(S_MODE3);
  #endif

  lastTime = millis(); // keep track of time

  // ISRs
	// ----
	attachISRs(); // Do that last so you won't risk stopping the "setup()"                                                  
}


// -------------------------- Loop --------------------------
void loop()
{

  // Take care of ISR flags - 1/55
  //----------------------------
  checkISRStates();

  // Check if a motor movement is needed (const speed) - 2/55
  //----------------------------------------------------------
  // (must be as fast as possible but slower than ISR)
  if (needPump) 
  {
    
    stepper.runSpeed(); // non blocking, cst speed, no accel

    if (stepper.distanceToGo() == 0) { // check if the pump movement is finished
      
      // bolean states
      needPump  = false; // for any pump movement (dose or prime)

      if (primeInProgress)
      {
        printDebugln("Priming done!");
        primeInProgress = false;
        isPrimed = true;
      }

      if (doseInProgress)
      {
        printDebugln("Dosing done!");
        doseInProgress = false;
      }

      

      // Just to be safe (not required) reset some boolean states
      needPrime = false;
      needDose  = false;

      printDebugln("Pump movement done!");

      disableStepper();

      // TODO: display how much time it took to perform the movement

      #ifdef USE_BUZZER
        // The motor stops
        cute.play(S_MODE3);
      #endif
    }
  } // if needPump


  // Check if we should prepare a primer or dose - 3/55
  //-----------------------------------------------------
  if (needPrime) // trumps a "dose" command
  {
    primePump(); // deals with the flags as well
  }
  else if(needDose)
  {
    dosePump(); // deals with the flags as well
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

  // pinMode(PIN_MOTOR_CFG_1, INPUT); // for HiZ
  // pinMode(PIN_MOTOR_CFG_2, OUTPUT);

  // //  digitalWrite(PIN_MOTOR_CFG_1,LOW); // 4/256 StealthChop
  // digitalWrite(PIN_MOTOR_CFG_2, HIGH); // 4/256 StealthChop



  // pinMode(PIN_MOTOR_CFG_1, OUTPUT);
  // pinMode(PIN_MOTOR_CFG_2, OUTPUT);

  // digitalWrite(PIN_MOTOR_CFG_1,LOW); // 1/N/A SpreadCycle
  // digitalWrite(PIN_MOTOR_CFG_2,LOW); // 1/N/A SpreadCycle



  //  pinMode(PIN_MOTOR_CFG_1, OUTPUT);
  // pinMode(PIN_MOTOR_CFG_2, OUTPUT);

  // digitalWrite(PIN_MOTOR_CFG_1,HIGH); // 16/N/A SpreadCycle
  // digitalWrite(PIN_MOTOR_CFG_2,HIGH); // 16/N/A SpreadCycle

  // 4/256 StealthChop
  pinMode(PIN_MOTOR_CFG_1, OUTPUT);
  pinMode(PIN_MOTOR_CFG_2, INPUT);
  digitalWrite(PIN_MOTOR_CFG_1,HIGH);
  //digitalWrite(PIN_MOTOR_CFG_2,HIGH);

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
void setupStepper(void) {


  #ifndef DISABLE_PUMP_AFTER_RUN 
  // if NOT define then we need to enable NOW (once at the start)
  enableStepper();
  #else
    // then nothing because the enable is going tio be before a movemnt
  #endif


  // Stepper set MAX limit values
  //------------------------------

  // To calculate max acceleration, max speed must be set first

  printDebug("Setting max SPEED to: ");
  printDebug(personalMaxSpeed * uStepFactor);
  printDebugln(" [micro steps/s]");
  stepper.setMaxSpeed(personalMaxSpeed * uStepFactor);

  printDebug("Setting max ACCEL to: ");
  printDebug(personalMaxAccel * uStepFactor);
  printDebugln(" [micro steps/s^2]");
  //stepper.setMaxSpeed(personalMaxAccel * uStepFactor);
  // No function "setMaxAcceleration()" available

  // Stepper accel + pos target
  //----------------------------
  
  // WE DONT WANT ANY ACCEL!!!!!!!!!

  //stepper.setAcceleration(targetAccel * uStepFactor);
  // do NOT set the speed with "setSpeed", it is ONLY for "runSpeed()"
  // stepper.moveTo(target_nbr_revolutions * pulsesPerRevolution * uStepFactor);  // absolute
  stepper.moveTo(0); // absolute


  displayStepperSettings(); // We do it once again after all the set up

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
  printDebug("Target position: ");
  printDebugln(stepper.targetPosition());

  printDebug("Current position: ");
  printDebugln(stepper.currentPosition());

  printDebug("Distance to go: ");
  printDebugln(stepper.distanceToGo());

  printDebugln("---------------------------------");

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

//******************************************************************************************
void checkISRStates(void) {
  
  if (isrFlag[INDX_BTN_CONT]==TRIGGERED || isrFlag[INDX_BTN_STRT]==TRIGGERED || isrFlag[PIN_BTN_ABRT]==TRIGGERED || isrFlag[INDX_BTN_RPT]==TRIGGERED)
  {
    Serial.println(millis());

    /////////////////// CONTINUE ///////////////////
    if(isrFlag[INDX_BTN_CONT]==TRIGGERED)
    {
      isrFlag[INDX_BTN_CONT]=UNTRIGGERED;// Reset the flag immediatly
      

      if (!primeInProgress && !doseInProgress) // if a action is happening, do not interrupt it
      {
        #ifdef USE_BUZZER
        cute.play(S_CONNECTION  ); // button pressed (Start-repeat-Continue)
        #endif
        printDebugln("Continue button pressed");
        printDebugln("No action associated with this button");
      }

    }
    /////////////////// START ///////////////////
    if(isrFlag[INDX_BTN_STRT]==TRIGGERED)
    {
      isrFlag[INDX_BTN_STRT]=UNTRIGGERED;// Reset the flag immediatly
      
      if (!primeInProgress && !doseInProgress) // if a action is happening, do not interrupt it
      {
        #ifdef USE_BUZZER
        cute.play(S_CONNECTION  ); // button pressed (Start-repeat-Continue)
        #endif
        printDebugln("Start button pressed");
        printDebugln("Priming asked");

        // Bolean states
        //--------------
        isPrimed  = false;
        //
        needPrime = true;
        needDose  = false;
        needPump  = false; // for any pump movement (dose or prime)
        primeInProgress = false; // just a safety
        doseInProgress = false;
      }

    }
    /////////////////// CONTINUE ///////////////////
    if(isrFlag[PIN_BTN_ABRT]==TRIGGERED)
    {
      isrFlag[PIN_BTN_ABRT]=UNTRIGGERED;// Reset the flag immediatly
      
      printDebugln("Abort button pressed");

      // Bolean states
      if(needPrime || primeInProgress)
      {
        // if we were priming then make sure we aknowledged the priming is NOT complete
        isPrimed = false;
      }

      // Bolean states
      //--------------
      needPrime = false; // reset the flag after check
      primeInProgress = false; // reset the flag after check
      needDose  = false;
      needPump  = false; // for any pump movement (dose or prime)
      doseInProgress = false;
      
      #ifdef USE_BUZZER
      cute.play(S_SAD           ); // Abort button presed
      #endif
    }
    /////////////////// REPEAT ///////////////////
    if(isrFlag[INDX_BTN_RPT]==TRIGGERED)
    {
      isrFlag[INDX_BTN_RPT]=UNTRIGGERED;// Reset the flag immediatly
      
      if (!primeInProgress && !doseInProgress) // if a action is happening, do not interrupt it
      {
        #ifdef USE_BUZZER
        cute.play(S_CONNECTION  ); // button pressed (Start-repeat-Continue)
        #endif
        printDebugln("Repeat button pressed");
        printDebugln("Dosing asked");

        // Bolean states
        //--------------
        // isPrimed  = false;
        //
        needPrime = false;
        needDose  = true;
        needPump  = false; // for any pump movement (dose or prime)
        primeInProgress = false; // just a safety
        doseInProgress = true;

      }

    }    
  }

} // END OF THE FUNCTION

//******************************************************************************************
void disableStepper(void) {
  printDebug("Disabling the motor...");
  // Disable the motor (inverse logic)
  digitalWrite(PIN_MOTOR_ENA, PUMP_DISABLE); // remove for more precision
  printDebugln("done");

  #ifdef USE_BUZZER
  cute.play(S_SLEEPING  ); // Disabling stepper
  #endif

} // END OF THE FUNCTION

//******************************************************************************************
void enableStepper(void) {
  printDebug("Enabling the motor...");
  // Enable the motor (inverse logic)
  digitalWrite(PIN_MOTOR_ENA, PUMP_ENABLE); // remove for more precision
  printDebugln("done");

  #ifdef USE_BUZZER
  cute.play(S_MODE1  ); // Enabling stepper
  #endif

  // // 0 Reference
  // //-------------------------
  // printDebug("Setting the \"0 position\" here");
  // stepper.setCurrentPosition(0u);// Has the side effect of setting the current motor speed to 0
  // printDebugln("done");


  // Stepper display settings
  //-------------------------
  displayStepperSettings();
  

} // END OF THE FUNCTION


//******************************************************************************************
void primePump(void) {
  
  printDebugln("Priming requested");

  #ifdef DISABLE_PUMP_AFTER_RUN 
  // if defined then we need to enable NOW (for each movement)
    enableStepper();
  #endif

  delay(1000);
  
  // // Set up stepper parameters
  // stepper.setMaxSpeed(1000);
  

  // Give stepper position goal
  printDebug("Setting the target position to ");
  printDebug(stepperDirection * (long)(floor(prime_target_nbr_revolutions * (float)((long)pulsesPerRevolution * (long)uStepFactor))));
  printDebug(" [micro steps] ... ");
  stepper.move(stepperDirection * (long)(floor(prime_target_nbr_revolutions * (float)((long)pulsesPerRevolution * (long)uStepFactor)))); // it won't move until we make a "run()"/"runSpeed()" call
  printDebugln("done");

  
  // If you are trying to use constant speed movements, you should call setSpeed() after calling moveTo().
  // stepper.setSpeed(doseSpeed);
  printDebug("Setting the constant speed to ");
  printDebug(primeSpeed * uStepFactor);
  printDebug(" [micro steps/s] ... ");
  stepper.setSpeed( primeSpeed * (float)(uStepFactor) );
  printDebugln("done");

  

  // set flags to repeatidly and regularly call runSpeed() until target position reached
  needPump = true;
  needPrime = false; // reset the flag
  primeInProgress = true;
  needDose = false;
  doseInProgress = false;

  printDebugln("And here... we... go... !");
  

} // END OF THE FUNCTION


//******************************************************************************************
void dosePump(void) {
  
  printDebugln("Dosing requested");


  #ifdef DISABLE_PUMP_AFTER_RUN 
  // if defined then we need to enable NOW (for each movement)
    enableStepper();
  #endif

  // delay(2000);
  
  // // Set up stepper parameters
  // stepper.setMaxSpeed(1000);
  

  // Give stepper position goal
  printDebug("Setting the target position to ");
  printDebug(stepperDirection * (long)(floor(dose_target_nbr_revolutions * (float)((long)pulsesPerRevolution * (long)uStepFactor))));
  printDebug(" [micro steps] ... ");
  stepper.move(stepperDirection * (long)(floor(dose_target_nbr_revolutions * (float)((long)pulsesPerRevolution * (long)uStepFactor)))); // it won't move until we make a "run()"/"runSpeed()" call
  printDebugln("done");

  
  // If you are trying to use constant speed movements, you should call setSpeed() after calling moveTo().
  // stepper.setSpeed(doseSpeed);
  printDebug("Setting the constant speed to ");
  printDebug(doseSpeed * uStepFactor);
  printDebug(" [micro steps/s] ... ");
  stepper.setSpeed( doseSpeed * (float)(uStepFactor) );
  printDebugln("done");

  

  // set flags to repeatidly and regularly call runSpeed() until target position reached
  needPump = true;
  needPrime = false;
  primeInProgress = false;
  needDose = false;// reset the flag
  doseInProgress = true;

  printDebugln("And here... we... go... !");
  

} // END OF THE FUNCTION


//END OF FILE