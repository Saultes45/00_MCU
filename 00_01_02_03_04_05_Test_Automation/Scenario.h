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

// -------------------------- Defines --------------------------
// Boolean states
bool    executingScenario   = false;      // Indicates if we are currently in a mixing loop

// Scenario
//----------

// A scenario is composed of differents funtions in a particular order

const int   nbr_movementsScenario           = 5; // This variable is const because it defines the size of the memory we can allocate to scenario parameters
long  scenarioSteps[nbr_movementsScenario]  = {2,3,4,5,6}; // this variable indicates the order in which we execute the different functions that make a dosing/mixing loop




// END OF THE FILE
