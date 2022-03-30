
#include <CuteBuzzerSounds.h>

#define BUZZER_PIN 8
#define NBR_LOOPS 4

int loopCounter = 0;

void setup() 
{
  

// Declare pins

cute.init(BUZZER_PIN);

// Console start
  Serial.begin(9600);
  while(!Serial) // Wait here until the connection with the computer has been established



// Set up starts
cute.play(S_OHOOH2);

delay(2000);

// Set up is done
cute.play(S_MODE3);


delay(2000);


 //start in manual mode
 cute.play(S_HAPPY); // Manual mode start

  
}


/* List of sounds needed [9]
* "Start"/"Repeat"/"Continue" button pressed 
* "Abort" button pressed 
* Set up start
* Set up ends
*
* "Manual" mode start
* "Auto"   mode start
*
* Ready for colorimeter test
* Done for colorimeter test
*
*Start of a new step (auto only)
*
*/


void loop() 
{


switch( loopCounter )
{

  cute.play(S_BUTTON_PUSHED ); // Start of a new step

  
  case 1:
      cute.play(S_CONNECTION  ); // button pressed (Start-repeat-Continue)
      cute.play(S_HAPPY         ); // Manual mode start
      break;
  case 2:
      cute.play(S_HAPPY_SHORT   ); // Auto mode start
      break;
  case 3:
      cute.play(S_MODE3         ); // Ready for colorimeter test AND Set up done
      delay(1000);
      cute.play(S_MODE3         ); // Ready for colorimeter test AND Set up done
      delay(1000);
      cute.play(S_MODE3         ); // Ready for colorimeter test AND Set up done
      delay(1000);
      cute.play(S_OHOOH2        ); // Done for colorimeter test  AND Set up start
      break;
   case 4:
    cute.play(S_SAD           ); // Abort button presed
  break;
  default:
      cute.play(S_SAD           ); // Abort button presed
      break;
}


// With ternary operator
  (loopCounter >= NBR_LOOPS-1) ?  loopCounter = 0 : loopCounter++;
  delay(3000);

  if (loopCounter == 0)
  {
    delay(5000);
    Serial.println("------------New cycle--------------");
  }


// All the following are blocking calls [7]
  
 
}
