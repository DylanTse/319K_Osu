// Osu.c
// Runs on TM4C123
// Dylan Tse & Pranav Kopparthy
// ECE319K Lab 10 Project
// Last Modified: 4/9/2023 

#include <stdint.h>
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
#define MID_COORD_Y 2100
#define MAX_COORD 4095
#define MAX_DELTA 500 // farthest mouse can move is a scalar of MAX_DELTA in a direction
#define PHYSICAL_DIAGONAL 205 // sqrt(128^2 + 160^2) = 204.80 --> map full joystick range to display

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
	uint32_t x;
	uint32_t y;
	arrow_t direction;
} sprite_t;


screenmode_t currScreen = START;
screenmode_t prevScreen = -1; // set OG value to invalid comparison
language_t langauge = ENGLISH;
difficulty_t difficulty = NORMAL;
uint32_t joystickData[2];
sprite_t mouse; // mouse = user cursor

/*****INTERRUPT HANDLERS*****/
void GPIOPortE_Handler(void){
	// [PE3: right] [PE2: left] [PE1: down] [PE0: up]
	GPIO_PORTE_ICR_R = 0x0F;
}

void GPIOPortC_Handler(void){
	// check PC6 (joystick 'IN' button)
	GPIO_PORTC_ICR_R = 0x40;
	currScreen = MENU;
}

void Timer1A_Handler(void){
  TIMER1_ICR_R = TIMER_ICR_TATOCINT; // acknowledge TIMER1A timeout
	JoyStick_In(joystickData);
	
	int32_t dx = joystickData[0] - MID_COORD_X;
	int32_t dy = joystickData[1]- MID_COORD_Y;
	int32_t scale_factor = (MAX_COORD / MAX_DELTA) + 1;
	
	// scaling so that based on how far push joystick
	dx /= scale_factor;
	dy /= scale_factor;
	int32_t mouseDX = (dx * PHYSICAL_DIAGONAL) / MAX_DELTA;
	int32_t mouseDY = (dy * PHYSICAL_DIAGONAL) / MAX_DELTA;
		
	// move mouse.x, mouse.y in direction of joystick	
	mouse.x += dx;
	mouse.y += dy;
}

void Timer0A_Handler(void){
	TIMER0_ICR_R = TIMER_ICR_TATOCINT;
}

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
	mouse.x = mouse.y = 2048; // default mouse value is in center of screen
	
	while(1){
		
		ST7735_DrawBitmap( (mouse.x*128)/4096, (mouse.y*160)/4096, cursor, 40, 40);
		
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
			prevScreen = currScreen;
		}
	}
}


void startScreen(void){
  ST7735_FillScreen(0x0000); // set screen to black
	ST7735_DrawBitmap(30,115, osuLogo, 70,70);
	ST7735_SetCursor(4,12);
	ST7735_OutString("Press joystick");
	ST7735_SetCursor(4,13);
	ST7735_OutString("down on Osu!");
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
