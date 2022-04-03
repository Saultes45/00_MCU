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
const uint8_t COLORIMETER_MEASURE_WAIT_S = 3; // how long to wait for the colorimeter to capture the data

// -------------------------- Global variables []----------------
// Sequencing
//------------
bool needPump 		= false;
bool nextFunction 	= false;

// -------------------------- Function declaration [8] ----------------
void      displayStepperSettings  (void);
void      pinSetUp                (void);

// Setups
void      setupConsole            (void);
void      setupValves             (void);
void      setupStepper            (void);
void      setupBuzzer             (void);
void      setupOLED               (void);

//OLED
void      printFunctionOLED       (void);
void      printChangeStepOLED     (void);
void      infoDisplayOLED         (void);

// Sequences
void      functionMeasure          (void); // OP
void      functionCleanASide       (void); // OP
void      functionCleanLoop        (void); // OP
void      functionMix              (void); // OP
void      functionDoseR1           (void); // OP
void      functionDoseR2           (void); // OP
void      functionMeasure          (void); // OP
void      functionCleanAir         (void); // Maintenence
void      functionCleanR1          (void); // Maintenence
void      functionCleanR2          (void); // Maintenence



// END OF THE FILE
