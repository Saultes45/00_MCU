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

// Stepper motor
#include <AccelStepper.h> // Mike McCauley https://www.airspayce.com/mikem/arduino/AccelStepper/

// -------------------------- Defines --------------------------


// Stepper motor driver pins (logic low)
//--------------------------------------
const uint8_t PIN_MOTOR_DIR    = 10; // Digital output 5V
const uint8_t PIN_MOTOR_STEP   = 11; // Digital output 5V
const uint8_t PIN_MOTOR_ENA    = 12; // Digital output 5V, manual logic


// Stepper conversion factors
//-----------------------------
/*
 * Handled exclusively by the motor driver - I know, it 
 * is NOT sorted ascendedly but what can I do?
 */
const long microSteppingFactorList[]                    = {1, 4, 8, 16, 32, 64, 128, 256, 5, 10, 20, 25, 40, 50, 100, 200}; 
/* Says to this program which physical microSteppingFactor 
 * is currently on the motor driver 
 */
const uint8_t indx_microSteppingFactorList              = 0;
const long    nativePulsesPerRevolution                 = ((long)200); // This parameter is from the motor and CANNOT be changed 
const long    microstepsPerRevolution                   = microSteppingFactorList[indx_microSteppingFactorList] * nativePulsesPerRevolution;

//Parameter for the different modes [4]
//-------------------------------------

//  Overall max
const float   max_allowedMicroStepsPerSeconds                   = 84000 * microSteppingFactorList[indx_microSteppingFactorList];
const float   max_allowedMicroStepsPerSecondsPerSeconds         = 84000.0 * 3.5 * microSteppingFactorList[indx_microSteppingFactorList]; // max speed achieved in 1/3.5 = 286 [ms]


// Mode 1 -  - no acceleration, slow speed
//const float   calibrationSpeedMicroStepsPerSecondsPerSeconds = 500.0;
const float   calibrationSpeedMicroStepsPerSeconds_max          = 100.0 * microSteppingFactorList[indx_microSteppingFactorList];
const float   calibrationSpeedMicroStepsPerSeconds_normal       = 100.0 * microSteppingFactorList[indx_microSteppingFactorList];
const long    calibrationExplorationMicroSteps                  = 1000 * microSteppingFactorList[indx_microSteppingFactorList]; // <DEBUG> too low
const long    detriggeringExplorationMicroSteps                 = 50 * microSteppingFactorList[indx_microSteppingFactorList]; // <DEBUG> too low

// Mode 3 - Single cycle movement - speed over acceleration, we DO need acceleration
const float   manualSpeedMicroStepsPerSecondsPerSeconds       = max_allowedMicroStepsPerSecondsPerSeconds;
const float   manualSpeedMicroStepsPerSeconds                 = max_allowedMicroStepsPerSeconds;



// -------------------------- Global variables ----------------


// Stepper motor driver 
//----------------------
/* Define a stepper and the pins it will use, and precise that
 *  we use an external dedicated stepper driver, thus we have 2 pins: 
 *  STEP and DIR (ENA is handled manually) 
 */
AccelStepper stepper(AccelStepper::DRIVER, PIN_MOTOR_STEP, PIN_MOTOR_DIR);

// boolean state (TODO: check if actually needed)
bool abortMovement = false;

// END OF THE FILE
