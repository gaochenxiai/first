#ifndef __LED_H
#define __LED_H	 
#include "sys.h"


#define LED0 PAout(15)	// PA5

//#define RD  PBout(3)	// 红光
//#define IRD PBout(4)	// 红外

void LED_Init(void);//初始化

#endif
