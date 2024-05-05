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




// -------------------------- Defines --------------------------

// Limit switches pins
//--------------------
const uint8_t PIN_SW_START      = 19;  // Digital input 5V - INT[2]
const uint8_t PIN_SW_CONTINUE   = 18;  // Digital input 5V - INT[3]
const uint8_t PIN_SW_STOP       = 2;  // Digital input 5V - INT[4]

const uint16_t SW_DEBOUNCE_TIME   = 250;    //100 // millisecond button debouncing time

const uint8_t PIN_LIMIT_RIGHT      = 0;  
const uint8_t PIN_LIMIT_LEFT       = 12;  


// -------------------------- Global variables ----------------

// States and timing
//-------------------

//START
volatile bool flagSW_start  = false; // this flag indicates a change to the main loop
//volatile bool flagSW_start = false; // this flag indicates if the LS is triggered to the main loop
volatile unsigned long last_interrupt_time_sw_start = 0;

//CONTINUE
volatile bool flagLS_l  = false; // this flag indicates a change to the main loop
//volatile bool stateLS_l = false; // this flag indicates if the LS is triggered to the main loop
volatile unsigned long last_interrupt_time_ls_l = 0;

//STOP
volatile bool flagSW_stop  = false; // this flag indicates a change to the main loop
//volatile bool flagSW_stop = false; // this flag indicates if the LS is triggered to the main loop
volatile unsigned long last_interrupt_time_sw_stop = 0;

volatile bool flagLS_r  = false;

volatile unsigned long last_interrupt_time_ls_r = 0;
volatile bool stateLS_r  = false;




// END OF THE FILE
