/* ========================================
*
* Copyright Nathanael Esnault, 2022
* All Rights Reserved
* UNPUBLISHED, LICENSED SOFTWARE.
*
* Metadata
* Written by    : Nathanael Esnault (Saultes45)
* Verified by   : N/A
* Creation date : 2022-04-03 
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


// -------------------------- Defines and Const []--------------------------


#define CONSOLE_BAUD_RATE             115200  // Baudrate in [bauds] for serial communication to the console



#define DEBOUNCE_TIME 400 // millisecond button debouncing time
#define NBR_BTN_ISR     4 // how many ISR-enabled buttons 

#define PIN_BTN_CONT 18 // Also TX1, ISR 5, Continue
#define PIN_BTN_STRT 19 // Also RX1, ISR 4, Start
#define PIN_BTN_ABRT 2  // ISR 0, Abort
#define PIN_BTN_RPT  3  // ISR 1, Repeat

#define INDX_BTN_CONT 0
#define INDX_BTN_STRT 1
#define INDX_BTN_ABRT 2
#define INDX_BTN_RPT  3


// ISR states
//-----------
#define TRIGGERED     true
#define UNTRIGGERED   false

// -------------------------- Global variables ----------------

volatile bool isrFlag[NBR_BTN_ISR] = {UNTRIGGERED};  // Start with no switch press pending, ie false (!triggered)

volatile unsigned long  last_interrupt_time[NBR_BTN_ISR] = {0u};

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

  pinMode(PIN_BTN_CONT, INPUT_PULLUP);
  pinMode(PIN_BTN_STRT, INPUT_PULLUP);
  pinMode(PIN_BTN_ABRT, INPUT_PULLUP);
  pinMode(PIN_BTN_RPT , INPUT_PULLUP);

  Serial.begin(CONSOLE_BAUD_RATE);

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
}


// -------------------------- Loop --------------------------
void loop()
{
  if (isrFlag[INDX_BTN_CONT]==TRIGGERED || isrFlag[INDX_BTN_STRT]==TRIGGERED || isrFlag[PIN_BTN_ABRT]==TRIGGERED || isrFlag[INDX_BTN_RPT]==TRIGGERED)
  {
    Serial.println(millis());
    if(isrFlag[INDX_BTN_CONT]==TRIGGERED)
    {
      isrFlag[INDX_BTN_CONT]=UNTRIGGERED;// Reset the flag immediatly
      Serial.println("Continue button pressed");
    }
    if(isrFlag[INDX_BTN_STRT]==TRIGGERED)
    {
      isrFlag[INDX_BTN_STRT]=UNTRIGGERED;// Reset the flag immediatly
      Serial.println("Start button pressed");
    }
    if(isrFlag[PIN_BTN_ABRT]==TRIGGERED)
    {
      isrFlag[PIN_BTN_ABRT]=UNTRIGGERED;// Reset the flag immediatly
      Serial.println("Abort button pressed");
    }
    if(isrFlag[INDX_BTN_RPT]==TRIGGERED)
    {
      isrFlag[INDX_BTN_RPT]=UNTRIGGERED;// Reset the flag immediatly
      Serial.println("Abort button pressed");
    }    
  }
}


//END OF FILE