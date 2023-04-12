// Timer0.c
// Runs on LM4F120/TM4C123
// Use TIMER0 in 32-bit periodic mode to request interrupts at a periodic rate
// Dylan Tse
// Last Modified: 4/11/2023

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

void (*PeriodicTask0)(void);
// ***************** Timer0_Init ****************
// Activate TIMER0 interrupts to run user task periodically
// Inputs:  period in units (1/clockfreq)
//          priority is 0 (high) to 7 (low)
// Outputs: none
void Timer0_Init(void(*task)(void), uint32_t period, uint32_t priority){
  volatile int delay;
  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
  delay = SYSCTL_RCGCTIMER_R;
	PeriodicTask0 = task;
  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER0_TAILR_R = period-1;    // 4) reload value
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|(priority<<29); // priority is bits 31,30,29
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
  TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
}


void Timer0A_Stop(void){
  TIMER0_CTL_R &= ~0x00000001; // disable, call to stop our sound
}

void Timer0A_Start(void){
  TIMER0_CTL_R |= 0x00000001; // enable, call to start our sound
}