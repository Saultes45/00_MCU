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

#include "Global.h"


// -------------------------- Defines --------------------------
// moved in "Global.h"

// -------------------------- Global variables ----------------
// moved in "Global.h"

// -------------------------- Functions declaration  --------------------------
// moved in "Global.h"





// -------------------------- SetUp --------------------------
void setup()
{

	// Debug communication channel UART through USB-B
	// -----------------------------------------------
	Serial.begin(CONSOLE_BAUD_RATE); //Begin serial communication (USB)
	#ifdef WAIT_FOR_SERIAL
	while (! Serial); // Wait for user to open the com port // <DEBUG> THIS IS ULTRA DANGEROUS
	#endif
	delay(1000);
	Serial.println("---------------------------------");

	// Declare pins
	// ------------
	pinSetUp();

  // Setting some boolean states
	// ---------------------------

	// ISRs
	// ----
	attachISRs();


	
} // END OF SET UP







// -------------------------- Loop --------------------------
void loop()
{


	
}// END OF THE LOOP



// END OF THE FILE
