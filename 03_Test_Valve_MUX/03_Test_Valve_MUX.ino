
/* ========================================
*
* Copyright Nathanael Esnault, 2022
* All Rights Reserved
* UNPUBLISHED, LICENSED SOFTWARE.
*
* Metadata
* Written by    : Nathanael Esnault (Saultes45)
* Verified by   : N/A
* Creation date : 2022-02-23 
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
*For tyhe MUX IC: search "PCF8574" in the library manager (+scroll down) 
* Author: Rob Tillaart
*
*
* TODO
*
*
* Terminology
* 
*
*/


// -------------------------- Includes [1] --------------------------
// Source: RobTillaart/PCF8574 (install via the manager but scroll down to #10, look at author)
#include "PCF8574.h" // https://www.arduino.cc/reference/en/libraries/pcf8574/

// -------------------------- Defines and Const []--------------------------

#define MUX_ADDRSS 0x20 

// Console
//-----------
#define SERIAL_VERBOSE // Comment out to remove message to the console (and save some exec time and power)
#define SERIAL_DEBUG // Uncomment to let the board print debug statements through the debug port (USB)
#define SERIAL_BAUDS 9600

// -------------------------- Global Variables [1]--------------------------
// 8-pin Mux for valve driver
PCF8574 VALVE_MUX(MUX_ADDRSS);

/*
+----------------+---------------------+-------------------------------------------------------+
| Side           |        Side B       |                         Side A                        |
+----------------+------+--------------+--------------+---------------+----+----+--------+-----+
| Position (dec) |   1  |       0      |       5      |       4       |  3 |  2 |    1   |  0  |
+----------------+------+--------------+--------------+---------------+----+----+--------+-----+
| MSB (hex)      |  128 |      64      |      32      |       16      |  8 |  4 |    2   |  1  |
+----------------+------+--------------+--------------+---------------+----+----+--------+-----+
| MSB (dec)      |   1  |       0      |       5      |       4       |  3 |  2 |    1   |  0  |
+----------------+------+--------------+--------------+---------------+----+----+--------+-----+
| Name           | Loop | Pump-Discard | Loop-Discard | Recirculation | R2 | R1 | Sample | Air |
+----------------+------+--------------+--------------+---------------+----+----+--------+-----+
*/
#define NBR_FUNCTIONS 6 // Also defines how many valves
//                                      F1 - F2 - F3 - F4 - F5 - F6
uint8_t valveMuxArray[NBR_FUNCTIONS] = {0x042,0xD2,0xA4,0xA8,0x90,0x00};

// Valves INDX
//-----------
// Side A
uint8_t valveAir            = 0;
uint8_t valveSample         = 1;
uint8_t valveR1             = 2;
uint8_t valveR2             = 3;
uint8_t valveRecirculation  = 4;
uint8_t valveLoopDiscard    = 5; 


// Side B
uint8_t valvePumpDiscard    = 6;
uint8_t valveLoop           = 7;

// For test purposes
//---------------------

uint8_t cnt_functions = 0;

// -------------------------- Macros [1] --------------------------
#ifdef SERIAL_DEBUG
#define printDebug(message)   Serial.print(message)
#define printDebugln(message) Serial.println(message)
#else
#define printDebug(message) // Leave the macro empty
#define printDebugln(message) // Leave the macro empty
#endif

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
  printDebugln("Welcome to the PCF8574 8-pin MUX test");



  printDebug("Initialisation...");
  printDebugln("done");

  printDebug("PCF8574_LIB_VERSION:\t");
  printDebugln(PCF8574_LIB_VERSION);


  printDebug("Trying to connect to PCF8574 at I2C address ");
  printDebug(MUX_ADDRSS);
  printDebug("(dec)...");
  if (!VALVE_MUX.begin())
  {
    printDebugln("PCF8574 could not be initialized...");
  }
  if (!VALVE_MUX.isConnected())
  {
    printDebugln("NOT connected");
  }
  else
  {
    printDebugln("connected");
  }



}


// -------------------------- Loop --------------------------
void loop() 
{
  //  // Write a value to the whole pins at once
  //  VALVE_MUX.write8(0xAA); // 0b 1010 1010
  //
  //  delay(1000);
  //
  //  VALVE_MUX.write8(0x55); // 0b 0101 0101
  //
  //  delay(1000);




  // Write a value to the whole pins at once
  printDebug("counter F");
  printDebug(cnt_functions+1);
  printDebug(": ");


  // Test a single valve (Bitmask)
  //------------------------------


//  int bitmask = (0x01) << valveRecirculation;
//  //  Bitwise ANDing in order to extract a subset of the bits in the value
//  //  Bitwise ORing in order to set a subset of the bits in the value
//  //  Bitwise XORing in order to toggle a subset of the bits in the value
//    printDebug("Bistmask for valve ");
//    printDebug(valveRecirculation);
//    printDebug(" is ");
//    printDebugln(bitmask);
//  
//  // Write a value, 1 pin at a time
//  //void PCF8574::write(const uint8_t pin, const uint8_t value)
//  VALVE_MUX.write(valveSample, (valveMuxArray[cnt_functions]) & bitmask);
//
//
//  printDebugln((valveMuxArray[cnt_functions]) & bitmask);


  // Test all valves at once
  //-------------------------

  VALVE_MUX.write8(valveMuxArray[cnt_functions]);
  printDebugln(valveMuxArray[cnt_functions]);

  // With ternary operator
  (cnt_functions >= NBR_FUNCTIONS-1) ?  cnt_functions = 0 : cnt_functions++;
  delay(1000);

  if (cnt_functions == 0)
  {
    delay(2000);
    printDebugln("------------New cycle--------------");
  }



}





// -------------------------- Functions definition [1]--------------------------






//******************************************************************************************
void      pinSetUp                (void)
{

  //  pinMode(LED, OUTPUT);



} // END OF FUNCTION









// END OF FILE
