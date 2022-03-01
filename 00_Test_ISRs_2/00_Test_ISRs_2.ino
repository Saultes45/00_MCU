

/* ========================================
*
* Copyright Nathanael Esnault, 2022
* All Rights Reserved
* UNPUBLISHED, LICENSED SOFTWARE.
*
* Metadata
* Written by    : Nathanael Esnault (Saultes45)
* Verified by   : N/A
* Creation date : 20??-??-?? 
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



#define SERIAL_VERBOSE // Comment out to remove message to the console (and save some exec time and power)


#define LED                         LED_BUILTIN  // digital pin connected to LED, for testing of switch code only

#define PIN_SW_CONT 18 // Also TX1, ISR 5, Continue
#define PIN_SW_STRT 19 // Also RX1, ISR 4, Start
#define PIN_SW_ABRT 2  // ISR 0, Abort
#define PIN_SW_RPT  3  // ISR 1, Repeat


#define switched                            true // value if the button switch has been pressed
#define triggered                           true // controls interrupt handler
#define interrupt_trigger_type              FALLING // interrupt triggered on a RISING input
#define debounce                            5 // time to wait, in [ms]

// ISR states
//-----------
#define TRIGGERED     true
#define UNTRIGGERED   false

// LOW to trigger the interrupt whenever the pin is low,
// CHANGE to trigger the interrupt whenever the pin changes value
// RISING to trigger when the pin goes from low to high,
// FALLING for when the pin goes from high to low.

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

// -------------------------- Global Variables [3]--------------------------
bool    led_status =                        LOW; // start with LED off, for testing of switch code only
//int     button_switch =                       2; // external interrupt pin


volatile bool initialisation_complete  = false;        // Inhibits any interrupts until initialisation is complete
volatile bool isrFlag                  = UNTRIGGERED;  // Start with no switch press pending, ie false (!triggered)


//volatile  bool interrupt_process_status = {
//  !triggered                                     // start with no switch press pending, ie false (!triggered)
//};




// -------------------------- Functions declaration [3] --------------------------
void      pinSetUp                (void);
void      attachISRs              (void);
bool      read_button             (void);



// -------------------------- ISR [1]--------------------------
//void button_interrupt_handler()
//{
//  if (initialisation_complete == true)
//  { //  all variables are initialised so we are okay to continue to process this interrupt
//    if (interrupt_process_status == !triggered) 
//    {
//      // new interrupt so okay start a new button read process -
//      // now need to wait for button release plus debounce period to elapse
//      // this will be done in the button_read function
//      if (digitalRead(button_switch) == LOW) 
//      {
//        // button pressed, so we can start the read on/off + debounce cycle wich will
//        // be completed by the button_read() function.
//        interrupt_process_status = triggered;  // keep this ISR 'quiet' until button read fully completed
//      }
//    }
//  }
//} // end of button_interrupt_handler


void button_interrupt_handler()
{
  if (initialisation_complete == true)
  { 
    isrFlag = TRIGGERED;
  }
} // end of button_interrupt_handler



// -------------------------- Set up --------------------------
void setup() 
{

  pinSetUp();
  

  

  #ifdef SERIAL_DEBUG
  Serial.begin(SERIAL_BAUDS);
  while(!Serial) // Wait here until the connection with the computer has been established
  #endif

  delay(1000);

  printDebugln("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
  printDebugln("Starting a new session");
  printDebugln("Welcome to the button ISR test");


  printDebug("Initialisation...");
  
  attachISRs();

  printDebugln("done");
}




// -------------------------- Loop --------------------------
void loop() 
{

  // test buton switch and process if pressed
//  if (read_button() == switched)
  if (isrFlag == TRIGGERED)  
  {
    isrFlag = UNTRIGGERED; //reset the flag
    printDebug("Flipping LED...");
    
//    // button on/off cycle now complete, so flip LED between HIGH and LOW
    led_status = HIGH - led_status; // toggle state
    digitalWrite(LED, led_status);

    printDebugln("done");
    
  } 
  else 
  {
    // do other things....
  }

}





// -------------------------- Functions definition [3]--------------------------



//******************************************************************************************
bool read_button() 
{
  
//  int button_reading;
//  
//  // static variables because we need to retain old values between function calls
//  static bool     switching_pending = false;
//  static long int elapse_timer;
//
//  if (interrupt_process_status == triggered) 
//  {
//    // interrupt has been raised on this button so now need to complete
//    // the button read process, ie wait until it has been released
//    // and debounce time elapsed
//    button_reading = digitalRead(PIN_SW_ABRT);
//    if (button_reading == HIGH) {
//      // switch is pressed, so start/restart wait for button relealse, plus end of debounce process
//      switching_pending = true;
//      elapse_timer = millis(); // start elapse timing for debounce checking
//    }
//    if (switching_pending && button_reading == LOW) {
//      // switch was pressed, now released, so check if debounce time elapsed
//      if (millis() - elapse_timer >= debounce) {
//        // dounce time elapsed, so switch press cycle complete
//        switching_pending = false;             // reset for next button press interrupt cycle
//        interrupt_process_status = !triggered; // reopen ISR for business now button on/off/debounce cycle complete
//        return switched;                       // advise that switch has been pressed
//      }
//    }
//  }

  return !switched; // either no press request or debounce period not elapsed

} // END OF FUNCTION


//******************************************************************************************
void      pinSetUp                (void)
{

  pinMode(LED, OUTPUT);
  //pinMode(button_switch, INPUT_PULLUP);

  pinMode(PIN_SW_CONT, INPUT_PULLUP);
  pinMode(PIN_SW_STRT, INPUT_PULLUP);
  pinMode(PIN_SW_ABRT, INPUT_PULLUP);
  pinMode(PIN_SW_RPT , INPUT_PULLUP);

} // END OF FUNCTION

//******************************************************************************************
void      attachISRs              (void)
{
  
//  attachInterrupt(digitalPinToInterrupt(button_switch),
//                    button_interrupt_handler,
//                    interrupt_trigger_type);

  attachInterrupt(digitalPinToInterrupt(PIN_SW_ABRT),
                    button_interrupt_handler,
                    interrupt_trigger_type);  

//  attachInterrupt(digitalPinToInterrupt(PIN_SW_CONT),
//                    ISR_SW_CONT,
//                    interrupt_trigger_type);          
//  attachInterrupt(digitalPinToInterrupt(PIN_SW_STRT),
//                    ISR_SW_STRT,
//                    interrupt_trigger_type);
//  attachInterrupt(digitalPinToInterrupt(PIN_SW_ABRT),
//                    ISR_SW_ABRT,
//                    interrupt_trigger_type);
//  attachInterrupt(digitalPinToInterrupt(PIN_SW_RPT),
//                    ISR_SW_RPT,
//                    interrupt_trigger_type);
                    
  
  initialisation_complete = true; // open interrupt processing for business

} // END OF FUNCTION





// END OF FILE
