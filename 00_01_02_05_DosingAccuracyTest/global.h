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
static const char codeVersion[]    = "0.1";  // current code version

// -------------------------- Global variables []----------------

// Time checking
//--------------
unsigned long lastTime    = 0u;
unsigned long currentTime = 0u;
unsigned long elapsedTime = 0u;

// -------------------------- Function declaration [8] ----------------
void      displayStepperSettings  (void);
void      pinSetUp                (void);

// Setups
void      setupConsole            (void);
void      setupStepper            (void);
void      setupBuzzer             (void);
void      setupOLED               (void);

//OLED
void      infoDisplayOLED         (void);

// Sequences
void      attachISRs              (void);
void      checkISRStates          (void);
void      primePump               (void);
void      dosePump                (void);

void      disableStepper          (void);
void      enableStepper           (void);



// END OF THE FILE
