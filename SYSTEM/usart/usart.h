#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 


#define USART1_REC_LEN  			200  	 //定义最大接收字节数 200

extern u8  USART1_RX_BUF[USART1_REC_LEN]; //最大USART_REC_LEN个字节.末字节为换行符 
extern u8  USART1_TX_BUF[USART1_REC_LEN]; //最大USART_REC_LEN个字节.末字节为换行符 
extern u16 Usart1_TX_Cou;//保存DMA每次数据发送的长度
extern u16 Usart1_RX_Cou;//保存DMA每次数据接收的长度
extern u8 Usart1_RX_Flag;//数据接收到后的空闲标志
void uart1_init(u32 bound);
void USART1_DMA_TX(u32 cmar, u16 cndtr);
void USART1_DMA_RX(void);


#endif


