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

// Source: RobTillaart/PCF8574 (install via the manager but scroll down to #10, look at author)
#include "PCF8574.h" // https://www.arduino.cc/reference/en/libraries/pcf8574/


// -------------------------- Defines and Const []--------------------------

#define MUX_ADDRSS 0x20 

// Valves INDX TODO: not actually used
// Side A
const uint8_t valveAir            = 0;
const uint8_t valveSample         = 1;
const uint8_t valveR1             = 2;
const uint8_t valveR2             = 3;
const uint8_t valveRecirculation  = 4;
const uint8_t valveLoopDiscard    = 5; 

// Side B
const uint8_t valvePumpDiscard    = 6; // might be inverted B0/B1
const uint8_t valveLoop           = 7; // might be inverted B0/B1

// -------------------------- Global variables []----------------

// I2C 8-pin mux for valve driver
PCF8574 VALVE_MUX(MUX_ADDRSS);


// END OF THE FILE
