#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 


#define USART1_REC_LEN  			200  	 //�����������ֽ��� 200

extern u8  USART1_RX_BUF[USART1_REC_LEN]; //���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u8  USART1_TX_BUF[USART1_REC_LEN]; //���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 Usart1_TX_Cou;//����DMAÿ�����ݷ��͵ĳ���
extern u16 Usart1_RX_Cou;//����DMAÿ�����ݽ��յĳ���
extern u8 Usart1_RX_Flag;//���ݽ��յ���Ŀ��б�־
void uart1_init(u32 bound);
void USART1_DMA_TX(u32 cmar, u16 cndtr);
void USART1_DMA_RX(void);


#endif


