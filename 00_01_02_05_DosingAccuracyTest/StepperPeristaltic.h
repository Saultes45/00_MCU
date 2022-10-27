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

#include <AccelStepper.h> // Mike McCauley


// -------------------------- Defines and Const [19]--------------------------

// Enable logic (active low)
//---------------------------
#define PUMP_ENABLE   LOW
#define PUMP_DISABLE  HIGH

// Movement pins
//--------------
const uint8_t PIN_MOTOR_STEP   = 11; // Digital output 5V
const uint8_t PIN_MOTOR_DIR    = 10; // Digital output 5V
const uint8_t PIN_MOTOR_ENA    = 12; // Digital output 5V, manual logic

// Parameters pins
//----------------
const uint8_t PIN_MOTOR_CFG_0   = 7; // Digital output 5V - Doesn't matter
const uint8_t PIN_MOTOR_CFG_1   = 4; // Digital output 5V
const uint8_t PIN_MOTOR_CFG_2   = 5; // Digital output 5V
const uint8_t PIN_MOTOR_CFG_3   = 6; // Digital output 5V - Doesn't matter

// To calculate position
//----------------------
const uint8_t pulsesPerRevolution = 200;  // Dictated by the HW only
const uint8_t uStepFactor         = 4;    // set up by the MCU on the stepper driver, // changes with the CFG1+2 config
const int interpolationFactor     = 256;            // changes with the CFG1+2 config
static const char driverMode[]    = "StealthChop";  // changes with the CFG1+2 config , "StealthChop"

// Personal MAX limits
//--------------------
const float personalMaxAccel = 900; //in [pulses/sec^2] so theo NOT dependent on uStepFactor
const float personalMaxSpeed = 900; //in [pulses/sec] so theo NOT dependent on uStepFactor

// Dose/Clean pump parameters
//---------------------------

// REMOVE THE NEXT LINE
const float targetAccel = personalMaxAccel; //in pulses/sec^2 so theo NOT dependent on uStepFactor
//const uint8_t targetSpeed = 100; //in pulses/sec so theo NOT dependent on uStepFactor

// Clean
//const float cleanSpeed = personalMaxSpeed; // Do not use, in [pulses/sec]
const float cleanAccel = personalMaxAccel; // in [pulses/sec^2]
// Mix
//const float mixSpeed = personalMaxSpeed; // Do not use, in [pulses/sec]
const float mixAccel = ((float)personalMaxAccel * 2.0/3.0); // in [pulses/sec^2]

//Dose
const float doseSpeed = 50.0;//10; //  in [pulses/sec]
// TODO: accel in that case is for DEBUG ONLY, it should be 0
// const float doseAccel = 150; // Do not use, in [pulses/sec^2]

//Prime
const float primeSpeed = 200.0; //  in [pulses/sec]
// TODO: accel in that case is for DEBUG ONLY, it should be 0
// const float doseAccel = 150; // Do not use, in [pulses/sec^2]

// -------------------------- Global variables [1]----------------

// Define a stepper and the pins it will use
AccelStepper stepper(AccelStepper::DRIVER, PIN_MOTOR_STEP, PIN_MOTOR_DIR);


// END OF THE FILE
