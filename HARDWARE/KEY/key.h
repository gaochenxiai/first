#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"


#define KEY0  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)//读取按键0


#define KEY0_PRES	1		//KEY0  

void KEY_Init(void);//IO初始化
u8 KEY_Scan(u8 mode);  	//按键扫描函数	

u8 KEY_Scan2(u8 mode);

#endif
