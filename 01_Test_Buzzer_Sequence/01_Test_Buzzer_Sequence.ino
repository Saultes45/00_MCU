
#include <CuteBuzzerSounds.h>

#define BUZZER_PIN 8

int loopCounter = 0;

void setup() 
{
  cute.init(BUZZER_PIN);

// Declare pins

// Console start

// Set up starts
cute.play(S_OHOOH2        ); // Done for colorimeter test  AND Set up start

delay(2000);

// Set up is done
cute.play(S_MODE3         );


delay(2000);


 //start in manual mode
 cute.play(S_HAPPY         ); // Manual mode start

  
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



// All the following are blocking calls [7]
  
  cute.play(S_CONNECTION  ); // button pressed (Start-repeat-Continue)
delay(2000);
//cute.play(S_DISCONNECTION )  ; // 
//delay(2000);
cute.play(S_BUTTON_PUSHED ); // Start of a new step
delay(2000);
//cute.play(S_MODE1         );
//delay(2000);
//cute.play(S_MODE2         );
//delay(2000);
cute.play(S_MODE3         ); // Ready for colorimeter test AND Set up done
delay(2000);
//cute.play(S_SURPRISE      ); 
//delay(2000);
//cute.play(S_OHOOH         );
//delay(2000);
cute.play(S_OHOOH2        ); // Done for colorimeter test  AND Set up start
delay(2000);
//cute.play(S_CUDDLY        );
//delay(2000);
//cute.play(S_SLEEPING      );
//delay(2000);
cute.play(S_HAPPY         ); // Manual mode start
delay(2000);
//cute.play(S_SUPER_HAPPY   );
//delay(2000);
cute.play(S_HAPPY_SHORT   ); // Auto mode start
delay(2000);
cute.play(S_SAD           ); // Abort button presed 
delay(2000);
cute.play(S_CONFUSED      );
delay(2000);
//cute.play(S_FART1         );
//delay(2000);
//cute.play(S_FART2         );
//delay(2000);
//cute.play(S_FART3         );
//delay(2000);
}
