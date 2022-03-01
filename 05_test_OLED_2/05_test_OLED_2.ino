



/* ========================================
*
* Copyright Nathanael Esnault, 2022
* All Rights Reserved
* UNPUBLISHED, LICENSED SOFTWARE.
*
* Metadata
* Written by    : Nathanael Esnault (Saultes45)
* Verified by   : N/A
* Creation date : 2022-02-22 
* Version       : 0.1 (finished on ...)
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
*
*
* TODO
*
*
* Terminology
* 
*
*/


/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/


// -------------------------- Includes [2] --------------------------
//#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

// -------------------------- Defines and Const []--------------------------



#define SERIAL_VERBOSE // Comment out to remove message to the console (and save some exec time and power)


#define LED                         LED_BUILTIN  // digital pin connected to LED, for testing of switch code only


// Console
//-----------
#define SERIAL_DEBUG // Uncomment to let the board print debug statements through the debug port (USB)
#define SERIAL_BAUDS 9600

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// -------------------------- Macros [1] --------------------------
#ifdef SERIAL_DEBUG
  #define printDebug(message)   Serial.print(message)
  #define printDebugln(message) Serial.println(message)
#else
  #define printDebug(message) // Leave the macro empty
  #define printDebugln(message) // Leave the macro empty
#endif

// -------------------------- Global Variables [0]--------------------------




// -------------------------- Functions declaration [0] --------------------------
void      pinSetUp                (void); // for template




// -------------------------- ISR [0]--------------------------



// -------------------------- Set up --------------------------
void setup() 
{

  pinSetUp();
  

  #ifdef SERIAL_DEBUG
  Serial.begin(SERIAL_BAUDS);
  while(!Serial) // Wait here until the connection with the computer has been established
  #endif

  delay(1000);

  printDebugln("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
  printDebugln("Starting a new session");
  printDebugln("Welcome to the SSD1302 test");
  


  printDebug("Initialisation...");

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    printDebugln(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();


  // Follow (2022-02-22)
  //https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/

  // Draw a single pixel in white
  //------------------------------
  display.drawPixel(10, 10, SSD1306_WHITE);
  // Show the display buffer on the OLED. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);
  // display.display() is NOT necessary after every single drawing command,
  // unless that's what you want...rather, you can batch up a bunch of
  // drawing operations and then update the screen all at once by calling
  // display.display(). These examples demonstrate both approaches...


  // Display text “Hello, world!”
  //------------------------------
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Hello, world!");
  display.display(); 


  //Draw a pixel
  display.drawPixel(64, 32, WHITE);

  //Draw a line
  display.drawLine(0, 0, 127, 20, WHITE);

  //Draw a rectangle
  //drawRect(x, y, width, height, color)
  display.drawRect(10, 10, 50, 30, WHITE);

  //fillRect(x, y, width, height, color)

  display.drawRoundRect(10, 10, 30, 50, 2, WHITE);

  display.fillRoundRect(10, 10, 30, 50, 2, WHITE);

  //Draw a circle
  display.drawCircle(64, 32, 10, WHITE);
  display.fillCircle(64, 32, 10, WHITE); 

  //Draw a triangle
  display.drawTriangle(10, 10, 55, 20, 5, 40, WHITE);

  display.fillTriangle(10, 10, 55, 20, 5, 40, WHITE);

  //Invert
  display.invertDisplay(true);
  //display.invertDisplay(true);

  display.display();

  printDebugln("done");
}




// -------------------------- Loop --------------------------
void loop() 
{

  

}





// -------------------------- Functions definition [1]--------------------------






//******************************************************************************************
void      pinSetUp                (void)
{

  pinMode(LED, OUTPUT);



} // END OF FUNCTION








// END OF FILE
