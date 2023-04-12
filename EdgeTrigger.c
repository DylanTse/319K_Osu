#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "EdgeTrigger.h"

void EdgeTrigger_Init(void){       
  SYSCTL_RCGCGPIO_R |= 0x00000010; // (a) activate clock for port E
  GPIO_PORTE_DIR_R &= ~0x0F;    	 // (c) make PE3-0 input
  GPIO_PORTE_DEN_R |= 0x0F;     	 //     enable digital I/O on PE3-0
  GPIO_PORTE_IS_R &= ~0x0F;     	 // (d) PE3-0 is edge-sensitive
  GPIO_PORTE_IBE_R &= ~0x0F;    	 //     PE3-0 is not both edges
  GPIO_PORTE_IEV_R |= 0x0F;    	 	 //     PE3-0 RISING edge event
  GPIO_PORTE_ICR_R = 0x0F;      	 // (e) clear flag4 *
  GPIO_PORTE_IM_R |= 0x0F;      	 // (f) arm interrupt on PE3-0
  NVIC_PRI1_R |= (5<<5); 					 // (g) priority 5 *
  NVIC_EN0_R = 0x00000010;      	 // (h) enable interrupt 4 in NVIC *
}

