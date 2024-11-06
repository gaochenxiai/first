#include "ct1711.h"

//初始化PB5和PE5为输出口.并使能这两个口的时钟		    
//LED IO初始化
void CT1711_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PA,PD端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 

}

void CT1711_reset(void)
{
  GPIO_ResetBits(GPIOB, GPIO_Pin_8);
  delay_us(500); // 500 us
  GPIO_SetBits(GPIOB, GPIO_Pin_8);

}

char CT1711_Read_Bit(void)
{
  char bi_data;
  GPIO_ResetBits(GPIOB, GPIO_Pin_8);
	delay_us(10); // 10 us  //这里比官方里程增加10us延时，没有此句话读不出数据
  GPIO_SetBits(GPIOB, GPIO_Pin_8);
  delay_us(20); // 20 us
  if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8))
  {
    bi_data = 1;
  } else {
    bi_data = 0;
  }
  
  GPIO_SetBits(GPIOB, GPIO_Pin_8);
  delay_us(30); // 30us
  
  return bi_data;
}

unsigned char CT1711_Read_Byte(void)
{
  unsigned char byte = 0;
  int i;
  for(i=8;i>0;i--)
  {
    byte <<= 1;
    byte |= CT1711_Read_Bit();
  }
  return byte;
}

float CT1711_Read_Temp_Degree(void)
{
  float temp = 0.00;
  unsigned char bit_cc0,bit_cc1,bit_sign;
  char temp_byte0,temp_byte1;
  int temp_val;
  
  CT1711_reset();
  delay_ms(150);//150
  bit_cc0 = CT1711_Read_Bit();
  delay_us(10);
  bit_cc1 = CT1711_Read_Bit();
  delay_us(10);
  bit_cc0 = bit_cc0&0x01;
  bit_cc1 = bit_cc1&0x01;
  if((bit_cc0 == 0x00) &&(bit_cc1 == 0x00))
  {
    bit_sign = CT1711_Read_Bit();
    delay_us(10);
    temp_byte0 = CT1711_Read_Byte();
    delay_us(10);
    temp_byte1 = CT1711_Read_Byte();
    delay_us(10);
    temp_val = (temp_byte0 << 8) + temp_byte1;
    if(bit_sign == 0x01)
    {
      temp_val = ~temp_val;
      temp_val &= 0xffff;
      temp_val++;
      temp = (-3.90625*temp_val/1000.00);
    } 
		else{
      temp = ((3.90625*(float)temp_val)/1000.00);
    }
    return temp;
  }
	return 0;
}



