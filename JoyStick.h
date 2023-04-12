#ifndef __JoyStick_h
#define __JoyStick_h
#include <stdint.h>

void JoyStick_Init(void);

void JoyStick_In(uint32_t data[2]);

#endif