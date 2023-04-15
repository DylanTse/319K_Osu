// Osu.c
// Runs on TM4C123
// Dylan Tse & Pranav Kopparthy
// ECE319K Lab 10 Project
// Last Modified: 4/9/2023 

#include <stdint.h>
#include <stdlib.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/ST7735.h"
#include "Random.h"
#include "TExaS.h"
#include "../inc/ADC.h"
#include "Images.h"
#include "../inc/wave.h"
#include "JoyStick.h"
#include "EdgeTrigger.h"
#include "Timer1.h"
#include "Timer0.h"


/*****PROTOTYPES*****/
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void startScreen(void);
void menuScreen(void);
void languageSelectScreen(void);
void difficultySelectScreen(void);
void gameScreen(void);
void pauseScreen(void);
void endScreen(void);


/*****GLOBALS + STRUCTS*****/
#define MID_COORD_X 2048
#define MID_COORD_Y 2048
#define MAX_COORD 4095
#define MAX_DELTA 400 // farthest mouse can move is a scalar of MAX_DELTA in a direction
#define PHYSICAL_DIAGONAL 205 // sqrt(128^2 + 160^2) = 204.80 --> map full joystick range to display

typedef enum {
	False, True
} bool;

typedef enum {
	START, MENU, LANG_SELECT, DIFF_SELECT, GAME, PAUSE, END
} screenmode_t;

typedef enum {
	ENGLISH, FRENCH
} language_t;

typedef enum {
	EASY, NORMAL, HARD
} difficulty_t;

typedef enum {
	UP, DOWN, LEFT, RIGHT
} arrow_t;

typedef struct {
	int32_t x;
	int32_t y;
	uint32_t width;
	uint32_t height;
} sprite_t;


screenmode_t currScreen = START;
screenmode_t prevScreen = -1; // set OG value to invalid comparison
language_t langauge = ENGLISH;
difficulty_t difficulty = NORMAL;
uint32_t joystickData[2];
arrow_t dirPressed = -1; // default value is invalid comparison

sprite_t mouse; // mouse = user cursor
sprite_t osuVals; //osu logo
sprite_t clearCursorVals; // need to clear the cursor area behind


/*****INTERRUPT HANDLERS*****/
void GPIOPortE_Handler(void){
	// [PE3: right] [PE2: left] [PE1: down] [PE0: up]
	GPIO_PORTE_ICR_R = 0x0F;
	switch(GPIO_PORTE_DATA_R){
		case 0x01: dirPressed = UP; break;
		case 0x02: dirPressed = DOWN; break;
		case 0x04: dirPressed = LEFT; break;
		case 0x08: dirPressed = RIGHT; break;
	}
}


void GPIOPortC_Handler(void){
	// check PC6 (joystick 'IN' button)
	GPIO_PORTC_ICR_R = 0x40;
	// if currently on Start screen --> then pressing in goes MENU. if playing and press in --> then pressing in goes PAUSE
	currScreen = (currScreen == START) ? MENU : PAUSE;
}


void Timer1A_Handler(void){
  TIMER1_ICR_R = TIMER_ICR_TATOCINT; // acknowledge TIMER1A timeout
	JoyStick_In(joystickData);
	
	int32_t dx = joystickData[0] - MID_COORD_X;
	dx = (abs(dx) > 50) ? dx : 0; // reduce no movement drifting
	int32_t dy = joystickData[1]- MID_COORD_Y;
	dy = (abs(dy) > 50) ? dy : 0;
	int32_t scale_factor = (MAX_COORD / MAX_DELTA) + 1;
	
	// scaling so that based on how far push joystick
	dx /= scale_factor;
	dy /= scale_factor;
	int32_t mouseDX = (dx * PHYSICAL_DIAGONAL) / MAX_DELTA;
	int32_t mouseDY = (dy * PHYSICAL_DIAGONAL) / MAX_DELTA;
	
	// move mouse.x, mouse.y in direction of joystick
	mouse.x *= 32; // convert 128x160 dimensions to 4096x4096 for pot
	mouse.y *= 26; // convert 128x160 dimensions to 4096x4096 for pot
	mouse.x += dx;
	mouse.y += dy;
	
	if( mouse.x <= 0){
		mouse.x = 0;
	} else if (mouse.x + (mouse.width*32) >= 4095){
		mouse.x = 4095 - (mouse.width*32);
	}
	
	if( mouse.y <= 0){
		mouse.y = (mouse.height*26);
	} else if (mouse.y >= 4095){
		mouse.y = 4095 - (mouse.height);
	}	
	
	mouse.x = (mouse.x*128)/4096;
	mouse.y = (mouse.y*159)/4096;
}



void Timer0A_Handler(void){
	TIMER0_ICR_R = TIMER_ICR_TATOCINT;
}



uint8_t hover(sprite_t cursor, sprite_t hitCircle);

/*****MAIN STUFF*****/
int main(void){
  DisableInterrupts();
	TExaS_Init(NONE); // Bus clock is 80 MHz
	Output_Init();
	ADC_Init();
	EdgeTrigger_Init();
	JoyStick_Init();
	Timer1_Init(80000000/30, 1); // sample at 30 Hz
	Wave_Init(); // init sound
	EnableInterrupts();
	
	// default values
	mouse.x = mouse.y = 65; // default mouse value is in center of screen
	mouse.width = mouse.height = clearCursorVals.width = clearCursorVals.height = 20;
	osuVals.x = 30; osuVals.y= 115;
	osuVals.width = osuVals.height = 70;
	clearCursorVals.x = clearCursorVals.y = 0; // start anywhere, will jump to prev cursor location
	
	while(1){
		
		ST7735_DrawBitmap( mouse.x, mouse.y, cursor, mouse.width, mouse.height);
		ST7735_DrawBitmap( mouse.x, mouse.y, clearCursor, mouse.width, mouse.height);
		
		if (currScreen != prevScreen){
			switch (currScreen){
				case START:
					startScreen();
					break;
				case MENU:
					menuScreen();
					break;
				case LANG_SELECT:
					languageSelectScreen();
					break;
				case DIFF_SELECT:
					difficultySelectScreen();
					break;
				case GAME:
					gameScreen();
					break;
				case PAUSE:
					pauseScreen();
					break;
				case END:
					endScreen();
					break;
			}
			//prevScreen = currScreen;
		}
		
	}
}


void startScreen(void){
  ST7735_FillScreen(0x0000); // set screen to black
	ST7735_DrawBitmap(osuVals.x, osuVals.y, osuLogo, osuVals.width, osuVals.height);
	ST7735_SetCursor(4,12);
	ST7735_OutString("Press joystick");
	ST7735_SetCursor(4,13);
	ST7735_OutString("down on Osu!");
	
	while(1){
		bool collide = hover(mouse, osuVals);
		if(collide == True){break;}
		ST7735_DrawBitmap( mouse.x, mouse.y, cursor, mouse.width, mouse.height);
	}	
	prevScreen = START;
	ST7735_OutString("HOVER SUCCESSFUL");
}

void menuScreen(void){
  ST7735_FillScreen(0x0000); // set screen to black
	ST7735_SetCursor(0,0);
	ST7735_DrawBitmap(30,115, Lang, 50,20);
}

void languageSelectScreen(void){}
void difficultySelectScreen(void){}
void gameScreen(void){}
void pauseScreen(void){}
void endScreen(void){}


bool valInRange(int32_t value, int32_t min, int32_t max){
	return (value >= min) && (value <= max);
}
	
uint8_t hover(sprite_t cursor, sprite_t hitCircle){
	bool xOverlap = False, yOverlap = False;
	
		// left edge, right edge, bottom edge, top edge
	int32_t rect1_coords[4] = { cursor.x, cursor.x+cursor.width-1, cursor.y, cursor.y+cursor.height+1};
	int32_t rect2_coords[4] = { hitCircle.x, hitCircle.x+hitCircle.width-1, hitCircle.y, hitCircle.y+hitCircle.height+1};

	// Possible cases to consider for no overlap to occur
	/*
		1) Rect1 left edge > Rect2 right edge 
		2) Rect1 right edge < Rect2 left edge 
		3) Rect1 bottom edge > Rect2 top edge
		4) Rect1 top edge < Rect2 bottom edge
	*/
	
	if (rect1_coords[0] > rect2_coords[1] || rect1_coords[1] < rect2_coords[0] ){
		return 0;
	}
	if (rect1_coords[2] < rect2_coords[3] || rect1_coords[3] > rect2_coords[2] ){
		return 0;
	}
	return 1;
}




/*
typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};

int main1(void){ char l;
  DisableInterrupts();
  TExaS_Init(NONE);       // Bus clock is 80 MHz 
  Output_Init();
  ST7735_FillScreen(0x0000);            // set screen to black
  for(phrase_t myPhrase=HELLO; myPhrase<= GOODBYE; myPhrase++){
    for(Language_t myL=English; myL<= French; myL++){
         ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString((char *)Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }  
}
*/
