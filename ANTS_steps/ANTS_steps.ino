


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


// -------------------------- Defines and Const []--------------------------

// -------------------------- Global Variables []--------------------------



bool bool_NeedStart = false;
bool bool_Started = false;

bool bool_NeedContinue = false;
bool bool_Continued = false;

bool bool_NeedStop = false;
bool bool_Stopped = true;



#define F_BAD         0
#define F_CLEAN_A     1
#define F_CLEAN_LOOP  2
#define F_DOSE_R1     3
#define F_DOSE_R2     4
#define F_MIX         5
#define F_MEAS        6

uint8_t currentFuntion = F_BAD;
uint8_t nextFuntion    = F_BAD;


// Timing for pump. Carefull, different speeds
#define WAIT_CLEAN_A_MS     2000
#define WAIT_CLEAN_LOOP_MS  2000
#define WAIT_DOSE_R1_MS     512 // Use a flowmeter for more dosing precision
#define WAIT_DOSE_R2_MS     512 // Use a flowmeter for more dosing precision
#define WAIT_MIX_MS         10500

// Not for MEAS since we wait for user input




// -------------------------- Functions declaration [0] --------------------------



// -------------------------- Set up --------------------------
void setup() 
{
  // put your setup code here, to run once:
}




// -------------------------- Loop --------------------------
void loop() 
{
  
  if (bool_NeedStart) // if the start button has been pressed
  {
    bool_NeedStart = false; //immediately reset the flag
    bool_Started = true;

    // Say what task needs to be next (clean A side)
    nextFuntion = F_CLEAN_A;


  }
  else // if the start button has NOT been pressed
  {
    if (bool_Started)
    {
      switch(currentFuntion) 
      {
        case F_BAD  :
          // Stop any currently running tests
          //-----------------------------------
          // disable stepper motor
          // close all valves

          // Set some variables
          //--------------------
          bool_NeedStart = false;
          bool_Started = false;

          bool_NeedContinue = false;
          bool_Continued = false;

          bool_NeedStop = false;// we are already stopped
          bool_Stopped = true; // we are already stopped

          //  Just keep waiting, need to reboot the MCU via RST push button or power cycle
          while(1);
          
          break; /* optional */

        case F_CLEAN_A  :
          // Set the valves

          // Configure the motor

          // Set the HW timer with ISR - do not start it yet

          // Start the motor 

          delay(WAIT_CLEAN_A_MS); // Just use a delay to simulate the ISR
          
          //Wait for the HW timer to fire the ISR

          // Stop the motor

          // Close the the valves

          // Prepare for next function
          nextFuntion = F_CLEAN_LOOP;
          
          break; /* optional */


        case F_CLEAN_LOOP  :
          // Set the valves

          // Configure the motor

          // Set the HW timer with ISR - do not start it yet

          // Start the motor 

          delay(WAIT_CLEAN_LOOP_MS); // Just use a delay to simulate the ISR
          
          //Wait for the HW timer to fire the ISR

          // Stop the motor

          // Close the the valves

          // Prepare for next function
          nextFuntion = F_DOSE_R1;
          
          break; /* optional */

        case F_DOSE_R1  :
          // Set the valves

          // Configure the motor

          // Set the FLOWMETER OR the HW timer with ISR - do not start it yet

          // Start the motor 

          delay(WAIT_DOSE_R1_MS);  // Just use a delay to simulate the ISR
          
          //Wait for the HW timer to fire the ISR

          // Stop the motor

          // Close the the valves

          // Prepare for next function
          nextFuntion = F_DOSE_R2;
          
          break; /* optional */

        case F_DOSE_R2  :
          // Set the valves

          // Configure the motor

          // Set the FLOWMETER OR the HW timer with ISR - do not start it yet

          // Start the motor 

          delay(WAIT_DOSE_R2_MS);  // Just use a delay to simulate the ISR
          
          //Wait for the HW timer to fire the ISR

          // Stop the motor

          // Close the the valves

          // Prepare for next function
          nextFuntion = F_MIX;
          
          break; /* optional */

        case F_MIX  :
          // Set the valves

          // Configure the motor

          // Set the FLOWMETER OR the HW timer with ISR - do not start it yet

          // Start the motor 

          delay(WAIT_MIX_MS);  // Just use a delay to simulate the ISR
          
          //Wait for the HW timer to fire the ISR

          // Stop the motor

          // Close the the valves

          // Prepare for next function
          nextFuntion = F_MEAS;          
          
          break; /* optional */ 

        case F_MEAS  :
          // NOTE: in the future we might want to do a mix (M) circulation when we measure
          
          // Make sure all the valves are closed

          // Make sure all the pump is stopped

          // Wait for the user to fire the ISR by pressintg the "CONTINUE" button 
          while( (!bool_NeedContinue) || (bool_NeedStop) ){}
          
          // If we come here, the user pressed either the CONTINUE or STOP button
          // TOD: what happens if he press the 
          
          // Reset the flag
          bool_NeedContinue = false;
          if (!bool_NeedStop)
          {
            bool_Continued = true;
          }
          


          // Say what task needs to be next (no need to clean A side)
          nextFuntion = F_CLEAN_LOOP;
          

          
          break; /* optional */


        default : /* Optional */
          // We should't go there

          // Do the same as F_BAD

          // Stop any currently running tests
          //-----------------------------------
          // disable stepper motor
          // close all valves

          // Set some variables
          //--------------------
          bool_NeedStart = false;
          bool_Started = false;

          bool_NeedContinue = false;
          bool_Continued = false;

          bool_NeedStop = false;// we are already stopped
          bool_Stopped = true; // we are already stopped

          //  Just keep waiting, need to reboot the MCU via RST push button or power cycle
          while(1);
        
      } // END SWITCH CASE
    } // END bool_Started == true

  }

// Now we take care of the loop cycle
nextFuntion = currentFuntion;



} // END OF LOOP





// -------------------------- Functions definition [0]--------------------------



//******************************************************************************************


// END OF FILE
