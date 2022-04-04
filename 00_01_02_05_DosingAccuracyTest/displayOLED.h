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

// -------------------------- Includes[2] --------------------------
#include <Wire.h>
#include <Adafruit_SSD1306.h>


// -------------------------- Defines and Const [5]--------------------------
#define USE_OLED // comment to NOT use the OLED

// Screen size
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// pins
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32


// -------------------------- Global variables [1]----------------
#ifdef USE_OLED
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif


// END OF THE FILE
