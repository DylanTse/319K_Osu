// Osu.c
// Runs on TM4C123
// Dylan Tse & Pranav Kopparthy
// ECE319K Lab 10 Project
// Last Modified: 4/14/2023 

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
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
double pi = 3.1415926535;

typedef enum {
	False, True
} bool;

typedef enum {
	START, LANG_SELECT, DIFF_SELECT, GAME, PAUSE, END
} screenmode_t;

typedef enum {
	ENGLISH, FRENCH
} language_t;

typedef enum {
	NORMAL, HARD
} difficulty_t;

typedef enum {
	UP, DOWN, LEFT, RIGHT
} arrow_t;

typedef struct {
	int32_t x;
	int32_t y;
	uint32_t width;
	uint32_t height;
	arrow_t direction;
	bool status; // if can hit or not hit
} sprite_t;


// CRITICAL INIT VALUES
language_t langauge = ENGLISH;
difficulty_t difficulty = NORMAL;
uint32_t joystickData[2];
arrow_t dirPressed = -1; // default value is invalid comparison

// SPRITE INITS
sprite_t mouse; // mouse = user cursor
sprite_t osuVals; //osu logo
sprite_t langPos;
sprite_t difficultyPos;
sprite_t menuPos;

// USER SPECIFIC MATERIAL
uint32_t score;
char* enMSG[] = {"PAUSED", "YOUR SCORE: ", "BYE!", "THX"};
char* frMSG[] = {"EN PAUSE", "TON SCORE: ", "SALUT!", "MERCI"};
screenmode_t option;



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



void Delay100ms(uint32_t count){
	uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
      time--;
    }
    count--;
  }
}

void GPIO_PortD_Init(void){
	volatile uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x0F;
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTD_DEN_R |= 0x43;
	GPIO_PORTD_DIR_R |= 0x43;
}




void menuSelectOption(void);
void drawSquare(int x, int y, const unsigned short *src);
void drawSquaresInCircle(int centerX, int centerY, int radius, int numSquares);
uint8_t hover(sprite_t cursor, sprite_t hitCircle);
/*****MAIN STUFF*****/
int main(void){
  DisableInterrupts();
	TExaS_Init(NONE); // Bus clock is 80 MHz
		Output_Init();
	ADC_Init();
		Sound_Init();
		Wave_Init(); // init sound
	EdgeTrigger_Init();
	JoyStick_Init();
	Timer1_Init(80000000/30, 1); // sample at 30 Hz
	Random_Init(21187); // random seed
	GPIO_PortD_Init();
	EnableInterrupts();
	
	// default values
	mouse.x = mouse.y = 20; // default mouse value is near center of screen
	mouse.width = mouse.height = 20;
	osuVals.x = 30; osuVals.y= 70;
	osuVals.width = osuVals.height = 70;
	langPos.x = 20; langPos.y = 100; langPos.width = 90; langPos.height = 29;
	difficultyPos.x = 20; difficultyPos.y = 140; difficultyPos.width = 90; difficultyPos.height = 29;
	menuPos.x = 20; menuPos.y = 125;
	menuPos.width=90; menuPos.height=29;

	startScreen();
	// wait for someone to choose menu option or play game
	while(1){
		menuSelectOption();
		if (option == LANG_SELECT){
			option = -1;
			languageSelectScreen();
			while(dirPressed != UP || dirPressed != DOWN){
				if (dirPressed == UP){langauge = ENGLISH; dirPressed = -1; break;}
				else if (dirPressed == DOWN){langauge = FRENCH; dirPressed = -1; break;}
			}
			startScreen();
		}
		
		else if (option == DIFF_SELECT){
			difficultySelectScreen();
			option = -1;
			while(dirPressed != UP || dirPressed != DOWN){
				if (dirPressed == UP){difficulty = NORMAL; dirPressed = -1; break;}
				else if (dirPressed == DOWN){difficulty = HARD; dirPressed = -1; break;}
			}
			startScreen();
		}
		
		else if (option == GAME){
			Wave_Hooray();
			Delay100ms(20);
			option = -1;
			ST7735_FillScreen(0x0000);
			Song_Init();
			ST7735_SetCursor(0,0);
				
			for(int i = 0; i < 7; i++){
				drawSquaresInCircle(43, 100, 47, 6);
			}
			
			endScreen();
			while(!hover(mouse, menuPos) || !joyButton){
				ST7735_DrawBitmap( mouse.x, mouse.y, cursor, mouse.width, mouse.height);
			}
			score = 0;
			startScreen();
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
	
	ST7735_DrawBitmap(langPos.x, langPos.y, Lang, langPos.width, langPos.height);
	ST7735_DrawBitmap(difficultyPos.x, difficultyPos.y, Difficulty, difficultyPos.width, difficultyPos.height);
}


void languageSelectScreen(void){
	ST7735_FillScreen(0x0000);
  ST7735_DrawBitmap(15, 60, ENlang, 100, 13);
	ST7735_DrawBitmap(15, 90, FRlang, 100, 13);
}

void difficultySelectScreen(void){
	ST7735_FillScreen(0x0000);
	if (langauge == ENGLISH){
		ST7735_SetCursor(3,6);
		ST7735_OutString( "UP for NORMAL" );
		ST7735_SetCursor(3,9);
		ST7735_OutString( "DOWN for HARD" );
	}
	else {
		ST7735_SetCursor(2,6);
		ST7735_OutString( "HAUT pour NORMAL" );
		ST7735_SetCursor(2,9);
		ST7735_OutString( "BAS pour DIFFICILE" );
	}
}
	
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
	if (langauge == ENGLISH){
		ST7735_SetCursor(2,6);
		ST7735_OutString("YOUR SCORE: ");
		ST7735_OutUDec(score);
	}
	else {
		ST7735_SetCursor(3,6);
		ST7735_OutString("TON SCORE: ");
		ST7735_OutUDec(score);
	}
	ST7735_DrawBitmap(20, 125, menuButton, 90, 29);
}



void menuSelectOption(void){
	while(!joyButton){
			GPIO_PORTD_DATA_R = 0x00;
			//ST7735_DrawBitmap( mouse.x, mouse.y, cursor, mouse.width, mouse.height);
			ST7735_DrawBitmap( mouse.x, mouse.y, cursor, mouse.width, mouse.height);
			if(hover(mouse, osuVals) && joyButton ){
				option = GAME;
				GPIO_PORTD_DATA_R = 0x02;
				Wave_Hit();
				GPIO_PORTD_DATA_R = 0x00;
				break;
			} 
			else if(hover(mouse, langPos) && joyButton ){
				option = LANG_SELECT;
				GPIO_PORTD_DATA_R = 0x02;
				Wave_Hit();
				break;
			}
			else if(hover(mouse, difficultyPos) && joyButton ){
				option = DIFF_SELECT;
				GPIO_PORTD_DATA_R = 0x02;
				Wave_Hit();
				break;
			}
	}
	joyButton = False;
}



// The following two functions are for displaying the hit squares in a circle pattern
void drawSquare(int x, int y, const unsigned short *src) {
	ST7735_DrawBitmap(x, y, src, 40, 40);
}

sprite_t hitSquare;
void drawSquaresInCircle(int centerX, int centerY, int radius, int numSquares) {
    // Calculate the angular increment between each square
    double angleIncrement = 2*pi/numSquares;
	
		int i = 0;
		while(i < numSquares){
			ST7735_DrawBitmap( mouse.x, mouse.y, cursor, mouse.width, mouse.height );
			
			if(hitCircleFlag == 2){
				
				double angle = i * angleIncrement;
        int x = centerX + (int)(radius * cos(angle));
        int y = centerY + (int)(radius * sin(angle));
				
				int rand = Random() % 4; //0,1,2,3
				switch(rand){
					case 0:
						drawSquare(x, y, mUP);
						hitSquare.x = x; hitSquare.y = y; hitSquare.height = hitSquare.width = 40; hitSquare.direction = UP; hitSquare.status = True;
						if(hover(mouse, hitSquare) && hitSquare.status){
							ST7735_FillRect(x, y-hitSquare.height, 40, 41, ST7735_BLACK);
							score++;
							dirPressed = -1;
							hitSquare.status = False;
						}
						break;
					case 1:
						drawSquare(x, y, mDOWN);
						hitSquare.x = x; hitSquare.y = y; hitSquare.height = hitSquare.width = 40; hitSquare.direction = DOWN; hitSquare.status = True;
						if(hover(mouse, hitSquare) && hitSquare.status){
							ST7735_FillRect(x, y-hitSquare.height, 40, 41, ST7735_BLACK);
							score++;
							dirPressed = -1;
							hitSquare.status = False;
						}
						break;
					case 2:
						drawSquare(x, y, mLEFT);
						hitSquare.x = x; hitSquare.y = y; hitSquare.height = hitSquare.width = 40; hitSquare.direction = LEFT; hitSquare.status = True;
						if(hover(mouse, hitSquare) && hitSquare.status){
							ST7735_FillRect(x, y-hitSquare.height, 40, 41, ST7735_BLACK);
							score++;
							dirPressed = -1;
							hitSquare.status = False;
						}
						break;
					case 3:
						drawSquare(x, y, mRIGHT);
						hitSquare.x = x; hitSquare.y = y; hitSquare.height = hitSquare.width = 40; hitSquare.direction = RIGHT; hitSquare.status = True;
						if(hover(mouse, hitSquare) && hitSquare.status){
							ST7735_FillRect(x, y-hitSquare.height, 40, 41, ST7735_BLACK);
							score++;
							dirPressed = -1;
							hitSquare.status = False;
						}
						break;
				}
				
				hitCircleFlag = 0;
				i++;
			}
		}
}


uint8_t hover(sprite_t cursor, sprite_t hitCircle){	
	// left edge, right edge, bottom edge, top edge
	int32_t rect1_coords[4] = { cursor.x, cursor.x+cursor.width-1, cursor.y, cursor.y-cursor.height+1};
	int32_t rect2_coords[4] = { hitCircle.x, hitCircle.x+hitCircle.width-1, hitCircle.y, hitCircle.y-hitCircle.height+1};
	
	if (rect1_coords[0] > rect2_coords[1] || rect1_coords[1] < rect2_coords[0] ){
		return 0;
	}
	if (rect1_coords[2] < rect2_coords[3] || rect1_coords[3] > rect2_coords[2] ){
		return 0;
	}
	return 1;
}
