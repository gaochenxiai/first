#include "key.h"
#include "delay.h"
#include "led.h"

//������ʼ������ 
//PA0.15��PC5 ���ó�����
void KEY_Init(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//ʹ��PORTA,PORTCʱ��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;//PA4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA4
}
//����������
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
//����ֵ��
//0��û���κΰ�������
//KEY0_PRES��KEY0����
//KEY1_PRES��KEY1����
//WKUP_PRES��WK_UP���� 
//ע��˺�������Ӧ���ȼ�,KEY0>KEY1>WK_UP!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(KEY0==0))
	{
		delay_ms(10);//ȥ���� 
		key_up=0;
		if(KEY0==0)return KEY0_PRES;
	}else if(KEY0==1)key_up=1; 	     
	return 0;// �ް�������
}
//��������100ms����
u8 KEY_Scan2(u8 mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(KEY0==0))
	{
		LED1=LED2=LED3=LED4=LED5=LED6=0;delay_ms(500);//ȥ���� 
		if(KEY0==0){
			LED6=1;delay_ms(500);//ȥ���� 
			if(KEY0==0){
				LED5=1;delay_ms(500);//ȥ���� 
				if(KEY0==0){
					LED4=1;delay_ms(500);//ȥ����
					if(KEY0==0){
						LED3=1;delay_ms(500);//ȥ���� 
						if(KEY0==0){
							LED2=1;delay_ms(500);//ȥ���� 
							if(KEY0==0){
								LED1=1;delay_ms(500);//ȥ����
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
	return 0;// �ް�������
}
