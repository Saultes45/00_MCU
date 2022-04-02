/* ========================================
*
* Copyright University of Auckland Ltd, 2022
* All Rights Reserved
* UNPUBLISHED, LICENSED SOFTWARE.
*
* Metadata
* Written by    : Nathanaël Esnault
* Verified by   : Nathanaël Esnault
* Creation date : 2022-04-01
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

// -------------------------- Includes[1] --------------------------


// -------------------------- Defines and Const []--------------------------



// -------------------------- Global variables []----------------
// Sequencing
//------------
bool needPump 		= false;
bool nextFunction 	= false;

// -------------------------- Function declaration [8] ----------------
void      displayStepperSettings  (void);
void      pinSetUp                (void);

void      setupValves             (void);
void      setupStepper            (void);
void      setupBuzzer             (void);
void      setupOLED               (void);

void      printFunctionOLED       (void);
void      printChangeStepOLED     (void);

void      functionMix             (void);



// END OF THE FILE
