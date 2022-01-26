/* ========================================
*
* Copyright >_FR4 design|, 2022
* All Rights Reserved
* UNPUBLISHED, LICENSED SOFTWARE.
*
* Metadata
* Written by    : Nathanaël Esnault
* Verified by   : Nathanaël Esnault
* Creation date : 2022-01-25
* Version       : 0.1 (finished on 2022-..-..)
* Modifications :
* Known bugs    :
*
*
* Possible Improvements
*
* Notes
*
*
* Ressources (Boards + Libraries Manager)
*
*
* TODO
*
* ========================================
*/


// -------------------------- Includes --------------------------

// Personal libraries
//--------------------
#include "StepperCalibration.h" 	// contains data for the translation between uL and uSteps
#include "Stepper.h" 				// Contains the data for the stepper speed, accel and pins
#include "Scenario.h" 				// Contains the variable for the sequencing of the functions (valve, pump, etc.)
#include "PushButtons.h" 				// Contains the variables for the 3 push buttons (Start/Continue/Stop) 

// -------------------------- Defines --------------------------

// General
//---------
#define SERIAL_VERBOSE                        // Uncomment to see more debug messages
#define WAIT_FOR_SERIAL                       // Uncomment to wait for the serial port to be opened from the PC end before starting
#define CONSOLE_BAUD_RATE             9600    // Baudrate in [bauds] for serial communication to the console (USB-B of the Arduino)

// -------------------------- Global variables ----------------



// -------------------------- Functions declaration [4] --------------------------
void      pinSetUp                (void);
void      attachISRs              (void);
void      enableStepper           (bool enableOrder);
void      displayStepperSettings  (void);



// -------------------------- ISR [1->3] ----------------

//******************************************************************************************
void LS_r_ISR()
{
	noInterrupts();
	unsigned long interrupt_time = millis();
	// If interrupts come faster than SW_DEBOUNCE_TIME, assume it's a bounce and ignore
	if (interrupt_time - last_interrupt_time_ls_r > SW_DEBOUNCE_TIME)
	{
		flagLS_r = true; // This indicates a change
    stateLS_r = digitalRead(PIN_LIMIT_RIGHT); // This is a SATE, not a FLAG!, the signal should be stable enough to read from the pin directly
		
    if(stateLS_r)
		{
      // Do stuff
		}

		last_interrupt_time_ls_r = interrupt_time;
	}

	interrupts();
}


/*------------------------------------------------------*/
/*-------------------- Functions [4] -------------------*/
/*------------------------------------------------------*/

//******************************************************************************************
void attachISRs (void)
{

#ifdef SERIAL_VERBOSE
	Serial.print("Attaching interrupts to their functions, be careful not to trigger anything...");
#endif
	/* Attach each push button a different ISR
*  For each we want to know when it is
*  triggered and released
*/
	attachInterrupt(digitalPinToInterrupt(PIN_LIMIT_RIGHT), LS_r_ISR, CHANGE);
	attachInterrupt(digitalPinToInterrupt(PIN_LIMIT_LEFT), LS_l_ISR, CHANGE);

  // Reset the time when the last LS interrupt was triggered (reference)
  last_interrupt_time_ls_r = millis();
  last_interrupt_time_ls_l = millis();

	// To trigger an action when the switch is pressed
	attachInterrupt(digitalPinToInterrupt(PIN_TOGGLE_MODE), toggleSwitchModeISR, CHANGE); 

#ifdef SERIAL_VERBOSE
	delay(250);
	Serial.println("done");
#endif

}// END OF THE FUNCTION

//******************************************************************************************
void pinSetUp (void)
{

#ifdef SERIAL_VERBOSE
	Serial.print("Setting up the pins...");
#endif

	// initialize digital pin PIN_MOTOR_ENA as an output
	pinMode(PIN_MOTOR_ENA, OUTPUT);

	// disable the motor immediatly
	enableStepper(false);

	// initialize the LED pin as an output:
	pinMode(LED_BUILTIN, OUTPUT);


#ifdef SERIAL_VERBOSE
	Serial.println("done");
#endif

}// END OF THE FUNCTION

//******************************************************************************************
void enableStepper(bool enableOrder)
{
	if (enableOrder)
	{
			#ifdef SERIAL_VERBOSE
			Serial.println("ENABLING the stepper, be careful...");
			#endif
			digitalWrite(PIN_MOTOR_ENA, LOW); // Inverse logic (active low)
			//    digitalWrite(PIN_MOTOR_ENA,HIGH); // Normal logic (active high)
	}
	else // in that case we want to be BLAZING FAST -> digitalWrite is BEFORE the Serial and TODO: used DMA instead
	{
		digitalWrite(PIN_MOTOR_ENA, HIGH); // Inverse logic (active low)
		//    digitalWrite(PIN_MOTOR_ENA,LOW); // Normal logic (active high)
		#ifdef SERIAL_VERBOSE
		Serial.print("DISABLING the stepper...");
		#endif

		#ifdef SERIAL_VERBOSE
		Serial.println("done");
		#endif
	}

}// END OF THE FUNCTION

//******************************************************************************************
void displayStepperSettings(void)
{
	// Display settings to UART
	//-------------------------
	Serial.println("---------------------------------");

	Serial.print("Current micro-steps settings: ");
	Serial.println(microSteppingFactorList[indx_microSteppingFactorList]);

	Serial.print("Current micro-steps per rotation: ");
	Serial.println(microstepsPerRevolution);

	Serial.println("---------------------------------");

	// To calculate max acceleration, max speed must be set first

	Serial.print("Current max speed: ");
	Serial.println(stepper.maxSpeed ()); // Returns the most recent speed in steps per second
	Serial.print("Setting max speed to: ");
	Serial.println(max_allowedMicroStepsPerSeconds); // Returns the most recent speed in [steps per second]
	stepper.setMaxSpeed(max_allowedMicroStepsPerSeconds);
	Serial.print("New max speed is: ");
	Serial.println(stepper.maxSpeed ()); // Returns the most recent speed in steps per second (float)

	Serial.print("Setting acceleration to: ");
	Serial.println(max_allowedMicroStepsPerSecondsPerSeconds);
	stepper.setAcceleration(max_allowedMicroStepsPerSecondsPerSeconds);
	// No function to read back set acceleration
	// NOTICE: Calling setAcceleration() is expensive, since it requires a square root to be calculated.

}// END OF THE FUNCTION



// END OF THE FILE
