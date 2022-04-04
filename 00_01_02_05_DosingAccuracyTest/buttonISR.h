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

// -------------------------- Includes[0] --------------------------



// -------------------------- Defines and Const [10]--------------------------

#define DEBOUNCE_TIME 400 // millisecond button debouncing time
#define NBR_BTN_ISR     4 // how many ISR-enabled buttons 

#define PIN_BTN_CONT 18 // Also TX1, ISR 5, Continue
#define PIN_BTN_STRT 19 // Also RX1, ISR 4, Start
#define PIN_BTN_ABRT 2  // ISR 0, Abort
#define PIN_BTN_RPT  3  // ISR 1, Repeat

#define INDX_BTN_CONT 0
#define INDX_BTN_STRT 1
#define INDX_BTN_ABRT 2
#define INDX_BTN_RPT  3

// ISR states
//-----------
#define TRIGGERED     true
#define UNTRIGGERED   false


// -------------------------- Global variables [2]----------------

volatile bool isrFlag[NBR_BTN_ISR] = {UNTRIGGERED};  // Start with no switch press pending, ie false (!triggered)

volatile unsigned long  last_interrupt_time[NBR_BTN_ISR] = {0u};

// END OF THE FILE
