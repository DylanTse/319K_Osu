// Osu.c
// Runs on TM4C123
// Dylan Tse & Pranav Kopparthy
// ECE319K Lab 10 Project
// Last Modified: 4/14/2023 

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
#include "Song.h"
#include "Sound.h"


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
#define MAX_DELTA 150 // farthest mouse can move is a scalar of MAX_DELTA in a direction

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


// CRITICAL INIT VALUES
screenmode_t currScreen = START;
screenmode_t prevScreen = -1; // set OG value to invalid comparison
language_t langauge = ENGLISH;
difficulty_t difficulty = NORMAL;
uint32_t joystickData[2];
arrow_t dirPressed = -1; // default value is invalid comparison

// SPRITE INITS
sprite_t mouse; // mouse = user cursor
sprite_t osuVals; //osu logo
sprite_t langPos;
sprite_t difficultyPos;

// USER SPECIFIC MATERIAL
uint32_t uScore;
const char *enMSG[4] = {"PAUSED", "YOUR SCORE: ", "BYE!", "THX"};
const char *frMSG[4] = {"EN PAUSE", "TON SCORE: ", "SALUT!", "MERCI"};



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

bool joyButton;
void GPIOPortC_Handler(void){
	// check PC6 (joystick 'IN' button)
	GPIO_PORTC_ICR_R = 0x40;
	joyButton = True;
}


void Timer1A_Handler(void){
  TIMER1_ICR_R = TIMER_ICR_TATOCINT; // acknowledge TIMER1A timeout
	JoyStick_In(joystickData);
	
	int32_t scale_factor = (MAX_COORD / MAX_DELTA) + 1;
	int32_t dx = joystickData[0] - MID_COORD_X;
	dx = (abs(dx) > 50) ? dx : 0; // reduce no movement drifting
	dx /= scale_factor;
	int32_t dy = joystickData[1]- MID_COORD_Y;
	dy = (abs(dy) > 50) ? dy : 0;
	dy /= scale_factor;
		
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
	mouse.y = (mouse.y*158)/4096;
}


uint8_t hover(sprite_t cursor, sprite_t hitCircle);
/*****MAIN STUFF*****/
int main(void){
  DisableInterrupts();
	TExaS_Init(NONE); // Bus clock is 80 MHz
		Output_Init();
	ADC_Init();
		Song_Init();
		Sound_Init();
		Wave_Init(); // init sound
	EdgeTrigger_Init();
	JoyStick_Init();
	Timer1_Init(80000000/30, 1); // sample at 30 Hz
	EnableInterrupts();
	
	// default values
	mouse.x = mouse.y = 20; // default mouse value is near center of screen
	mouse.width = mouse.height = 20;
	osuVals.x = 30; osuVals.y= 115;
	osuVals.width = osuVals.height = 70;
	langPos.x = 20; langPos.y = 60; langPos.width = 90; langPos.height = 29;
	difficultyPos.x = 20; difficultyPos.y = 115; difficultyPos.width = 90; difficultyPos.height = 29;
	
	Wave_Hit();
	Song_Init();
	
	startScreen();
	// busy wait until cursor over and click on Osu logo
	while( !hover(mouse, osuVals) || !joyButton){
			ST7735_DrawBitmap( mouse.x, mouse.y, cursor, mouse.width, mouse.height);
	}
	menuScreen();
	joyButton = False;
	
	screenmode_t option;
	while(!joyButton){
			ST7735_DrawBitmap( mouse.x, mouse.y, cursor, mouse.width, mouse.height);
			if(hover(mouse, langPos) && joyButton ){
				option = LANG_SELECT;
				break;
			}
	}
	
	if (option == LANG_SELECT){
		languageSelectScreen();
		while(dirPressed != UP || dirPressed != DOWN){
			if (dirPressed == UP){langauge = ENGLISH; break;}
			else if (dirPressed == DOWN){langauge = FRENCH; break;}
		}
	}
	
	
}



/*****SCREENS + HELPER FUNCTIONS*****/
void startScreen(void){
  ST7735_FillScreen(0x0000); // set screen to black
	ST7735_DrawBitmap(osuVals.x, osuVals.y, osuLogo, osuVals.width, osuVals.height);
	ST7735_SetCursor(4,12);
	ST7735_OutString("Press joystick");
	ST7735_SetCursor(4,13);
	ST7735_OutString("down on Osu!");
}

void menuScreen(void){	
  ST7735_FillScreen(0x0000); // set screen to black
	ST7735_SetCursor(0,0);
	ST7735_DrawBitmap(langPos.x, langPos.y, Lang, langPos.width, langPos.height);
	ST7735_DrawBitmap(difficultyPos.x, difficultyPos.y, Difficulty, difficultyPos.width, difficultyPos.height);
}

void languageSelectScreen(void){
	ST7735_FillScreen(0x0000);
  ST7735_DrawBitmap(15, 60, ENlang, 100, 13);
	ST7735_DrawBitmap(15, 90, FRlang, 100, 13);
}

void difficultySelectScreen(void){}
void gameScreen(void){}
	
void pauseScreen(void){
	ST7735_FillScreen(0x0000);
	ST7735_SetCursor(2,5);
	ST7735_OutString("PAUSED");
	ST7735_SetCursor(2,7);
	ST7735_OutString("Press any key");
	ST7735_SetCursor(2,8);
	ST7735_OutString("to resume");
}
	
void endScreen(void){
	ST7735_FillScreen(0x0000);
	ST7735_SetCursor(2,5);
	ST7735_OutUDec(uScore);
}


uint8_t hover(sprite_t cursor, sprite_t hitCircle){	
	// left edge, right edge, bottom edge, top edge
	int32_t rect1_coords[4] = { cursor.x, cursor.x+cursor.width-1, cursor.y+cursor.height-1, cursor.y};
	int32_t rect2_coords[4] = { hitCircle.x, hitCircle.x+hitCircle.width-1, hitCircle.y+hitCircle.height-1, hitCircle.y};
	if (rect1_coords[0] > rect2_coords[1] || rect1_coords[1] < rect2_coords[0] ){
		return 0;
	}
	if (rect1_coords[2] < rect2_coords[3] || rect1_coords[3] > rect2_coords[2] ){
		return 0;
	}
	return 1;
}
