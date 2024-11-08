#include "key.h"
#include "delay.h"
#include "led.h"

//按键初始化函数 
//PA0.15和PC5 设置成输入
void KEY_Init(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能PORTA,PORTC时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;//PA4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA4
}
//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//返回值：
//0，没有任何按键按下
//KEY0_PRES，KEY0按下
//KEY1_PRES，KEY1按下
//WKUP_PRES，WK_UP按下 
//注意此函数有响应优先级,KEY0>KEY1>WK_UP!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY0==0))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY0==0)return KEY0_PRES;
	}else if(KEY0==1)key_up=1; 	     
	return 0;// 无按键按下
}
//长按按键100ms以上
u8 KEY_Scan2(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY0==0))
	{
		LED1=LED2=LED3=LED4=LED5=LED6=0;delay_ms(500);//去抖动 
		if(KEY0==0){
			LED6=1;delay_ms(500);//去抖动 
			if(KEY0==0){
				LED5=1;delay_ms(500);//去抖动 
				if(KEY0==0){
					LED4=1;delay_ms(500);//去抖动
					if(KEY0==0){
						LED3=1;delay_ms(500);//去抖动 
						if(KEY0==0){
							LED2=1;delay_ms(500);//去抖动 
							if(KEY0==0){
								LED1=1;delay_ms(500);//去抖动
								if(KEY0==0){	
									key_up=0;
									if(KEY0==0)return KEY0_PRES;
								}									
							}
						}
					}
				}
			}
		}
	}
	else if(KEY0==1)key_up=1; 	     
	return 0;// 无按键按下
}
