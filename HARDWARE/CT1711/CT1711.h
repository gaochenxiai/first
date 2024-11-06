#ifndef __CT1711_H
#define __CT1711_H	 
#include "sys.h"



void CT1711_Init(void);//≥ı ºªØ
void CT1711_reset(void);
char CT1711_Read_Bit(void);
unsigned char CT1711_Read_Byte(void);
float CT1711_Read_Temp_Degree(void);

#endif
