// Sound.c
// This module contains the SysTick ISR that plays sound
// Runs on TM4C123
// Program written by: Dylan Tse, Pranav Kopparthy
// Date Created: 3/6/17 
// Last Modified: 2/26/23 
// Lab number: 6

// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data
#include <stdint.h>
#include "../inc/dac.h"
#include "../inc/tm4c123gh6pm.h"

uint8_t index;
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts


const uint8_t SinWave[64] = {				
  32,35,38,41,44,47,49,52,54,56,58,				
  59,61,62,62,63,63,63,62,62,61,59,				
  58,56,54,52,49,47,44,41,38,35,				
  32,29,26,23,20,17,15,12,10,8,				
  6,5,3,2,2,1,1,1,2,2,3,				
  5,6,8,10,12,15,17,20,23,26,29
};			

const uint8_t CoolGuitar[64] = {  
  29,29,29,28,25,22,18,14,13,15,20,
  26,35,42,47,50,51,49,45,38,30,23,
  19,19,24,32,43,54,60,62,59,54,
  47,40,35,32,30,29,29,29,31,34,
  36,38,39,38,36,31,27,23,21,20,
  21,23,23,23,23,23,24,25,27,29,29,29
};

const uint8_t Flute[64] = {  	
  31,36,40,42,44,47,50,53,56,57,59,
  60,60,60,59,57,55,53,51,48,45,42,
  39,37,34,33,31,30,28,28,27,26,
  25,26,26,25,25,24,23,23,22,21,
  20,18,17,16,15,15,14,13,13,13,
  13,14,15,16,17,18,19,21,23,25,27,29
};  


// **************Sound_Init*********************
// Initialize digital outputs and SysTick timer
// Called once, with sound/interrupts initially off
// Input: none
// Output: none
void Sound_Init(void){
	DAC_Init();
	index = 0;
	NVIC_ST_CTRL_R = 0; // disable SysTick during setup
	NVIC_ST_CURRENT_R = 0; // clear current
	NVIC_ST_CTRL_R = 0x0007; // enable SysTick with core clock + interrupts
}


// **************Sound_Start*********************
// Start sound output, and set Systick interrupt period 
// Sound continues until Sound_Start called again, or Sound_Off is called
// This function returns right away and sound is produced using a periodic interrupt
// Input: interrupt period
//           Units of period to be determined by YOU
//           Maximum period to be determined by YOU
//           Minimum period to be determined by YOU
//         if period equals zero, disable sound output
// Output: none
void Sound_Start(uint32_t period){
	EnableInterrupts();
  NVIC_ST_RELOAD_R = period-1;// reload value
}



// **************Sound_Voice*********************
// Change voice
// EE319K optional
// Input: voice specifies which waveform to play
//           Pointer to wave table
// Output: none
void Sound_Voice(const uint8_t *voice){
  // optional
}
// **************Sound_Off*********************
// stop outputing to DAC
// Output: none
void Sound_Off(void){
	DisableInterrupts();
}


// **************Sound_GetVoice*********************
// Read the current voice
// EE319K optional
// Input: 
// Output: voice specifies which waveform to play
//           Pointer to current wavetable
const uint8_t *Sound_GetVoice(void){
  // if needed
  return 0; // replace
}
#define PF4 (*((volatile uint32_t *)0x40025040))
#define PF3 (*((volatile uint32_t *)0x40025020))
#define PF2 (*((volatile uint32_t *)0x40025010))
#define PF1 (*((volatile uint32_t *)0x40025008))
#define PF0 (*((volatile uint32_t *)0x40025004))

// Interrupt service routine
// Executed every 12.5ns*(period)
void SysTick_Handler(void){
	
	index = (index + 1) & 0x3F;
	DAC_Out(Flute[index]);
	
}



