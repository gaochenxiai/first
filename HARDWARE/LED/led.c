#include "led.h"

//��ʼ��PB5��PE5Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PA,PD�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;				 //LED0-->PA.5 �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA.5
	GPIO_SetBits(GPIOA,GPIO_Pin_5);						 //PA.5 �����

//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //LED0-->PA.5 �˿�����
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; 		 //�������
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
//	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA.5
	
//	GPIO_ResetBits(GPIOB,GPIO_Pin_3);						 //PA.5 �����
//	GPIO_ResetBits(GPIOB,GPIO_Pin_4);						 //PA.5 �����
}
