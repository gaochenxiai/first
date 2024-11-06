#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

extern u32 led_flag;
extern u8 time5s;
void TIM3_Int_Init(u16 arr,u16 psc); 



#endif
