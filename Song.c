// Song.c
// Lab 6 Extra credit 1) 
// playing your favorite song.
//
// For use with the TM4C123
// ECE319K lab6 extra credit 1)
// Program written by: put your names here
// 1/2/23

#include "Sound.h"
#include "../inc/DAC.h"
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Song_Play(void);
void Timer3A_Init( void(*task)(void), uint32_t period, uint32_t priority);
void Timer3A_Stop(void);

short hitCircleFlag;

// TWINKLE TWINKLE LITTLE STAR
const uint16_t Star[42] = {
2389, 2389, 3189, 3189, 2841, 2841, 3189,
3579, 3579, 3792, 3792, 4257, 4257, 2389,
3189, 3189, 3579, 3579, 3792, 3792, 4257,
3189, 3189, 3579, 3579, 3792, 3792, 4257,
2389, 2389, 3189, 3189, 2841, 2841, 3189,
3579, 3579, 3792, 3792, 4257, 4257, 2389
};

uint8_t i;


void Song_Init(void){
  i = 0;
	hitCircleFlag = 0;
	EnableInterrupts();
	Timer3A_Init(Song_Play, 33321450, 2);
	
}

// Play song
void Song_Play(void){
  Sound_Start(Star[i]);
	i = (i + 1) % 42;
	hitCircleFlag += 1;
}

// Stop song
void Song_Stop(void){
	Timer3A_Stop();
	DisableInterrupts();
}

