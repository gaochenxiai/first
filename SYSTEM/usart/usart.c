#include "sys.h"
#include "usart.h"	 

////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 







u8 USART1_RX_BUF[USART1_REC_LEN];     //���USART_REC_LEN���ֽ�.
u8 USART1_TX_BUF[USART1_REC_LEN];     //���USART_REC_LEN���ֽ�.
u16 Usart1_TX_Cou;             //����DMAÿ�����ݷ��͵ĳ���
u16 Usart1_RX_Cou = 0;
u8 Usart1_RX_Flag = 0;
//��ʼ��IO ����1 
//bound:������
void uart1_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  USART_DeInit(USART1);  //��λ����1

	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART1_RX	  GPIOA.10��ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//�����ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	USART_ITConfig(USART1,USART_IT_TC,DISABLE); 		//�رշ�������ж�
	USART_ITConfig(USART1,USART_IT_RXNE,DISABLE);		//�رս�������ж�
	USART_ITConfig(USART1,USART_IT_IDLE,ENABLE);    //�������տ����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 
}
//��ҪҪ����Ϊ�������ڷ���Ҫ���������*�ŵ�
//DMA1�ĸ�ͨ������
//����Ĵ�����ʽ�ǹ̶���,���Ҫ���ݲ�ͬ��������޸�
//�Ӵ洢��->����ģʽ/8λ���ݿ��/�洢������ģʽ
//cmar:�洢����ַ�����綨��u8 Usart1_TX[10];������д(u32)Usart1_TX�� ������͵���������Ҫ��(u32)&i�����iռ2�ֽ� ��ôcndtr��Ҫ��2
//cndtr:���ݴ����� 
void USART1_DMA_TX(u32 cmar, u16 cndtr)
{
 	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMAʱ��
	
	DMA_DeInit(DMA1_Channel4);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ*
	Usart1_TX_Cou=cndtr;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;  //DMA����ADC����ַ,*
	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //���ݴ��䷽�򣬴��ڴ��ȡ���͵�����
	DMA_InitStructure.DMA_BufferSize = cndtr;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //��������������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���*
	
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //ʹ�ܴ���1��DMA����*
	DMA_Cmd(DMA1_Channel4, DISABLE );  //�ر�USART1 TX DMA1 ��ָʾ��ͨ��*
	DMA_SetCurrDataCounter(DMA1_Channel4,cndtr);//DMAͨ����DMA����Ĵ�С*
	DMA_Cmd(DMA1_Channel4, ENABLE);  //ʹ��USART1 TX DMA1 ��ָʾ��ͨ�� *
} 
void USART1_DMA_RX(void)
{
	//������DMA����
	DMA_InitTypeDef DMA_InitStructure;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//����DMAʱ��
	
	DMA_DeInit(DMA1_Channel5);   //DMA1ͨ��5����
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);   //�����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART1_RX_BUF;   //�ڴ��ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;        //dma���䷽����
	DMA_InitStructure.DMA_BufferSize = USART1_REC_LEN;      
	//����DMA�ڴ���ʱ�������ĳ��ȣ����ֵȡԤ�ƽ��յ��������
	//����Usart1_RX_LENΪ512�������յ�һ���ֽں���u32 i=DMA_GetCurrDataCounter(DMA1_Channel5); ����i��ֵ����511��
	//��ô��u32 Cou = Usart1_RX_LEN - DMA_GetCurrDataCounter(DMA1_Channel5);�͵õ��˽����ֽڵļ�������ôCou = 1;
	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 		//����DMA���������ģʽ��һ������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  	 //����DMA���ڴ����ģʽ
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//���������ֳ�
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 	//�ڴ������ֳ�
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  				//����DMA�Ĵ���ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; //����DMA�����ȼ���
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;			 //����DMA��2��memory�еı����������
	DMA_Init(DMA1_Channel5,&DMA_InitStructure);

	DMA_Cmd(DMA1_Channel5,ENABLE);	//ʹ��ͨ��5
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);		//����DMA��ʽ����
}
//����1����֡�ж�   
void USART1_IRQHandler(void)                               
{   
	uint32_t temp = 0;
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  //�ж��Ƿ���Ŀ���֡����
    {
    	//USART_ClearFlag(USART1,USART_IT_IDLE);
    	temp = USART1->SR;	//�ȶ�SR��Ȼ���DR�������
    	temp = USART1->DR; //��USART_IT_IDLE��־
    	DMA_Cmd(DMA1_Channel5,DISABLE);   //�ر�DMA���գ���ֹ��仹�����ݹ���
			temp = USART1_REC_LEN - DMA_GetCurrDataCounter(DMA1_Channel5); //������յ��ֽ���
			Usart1_RX_Cou = temp;    //������յ����ֽ���,main�д����Ҫ����
			Usart1_RX_Flag = 1;				//����1һ�����ݰ�������ɣ���־��һ��main�д����Ҫ����
			DMA_SetCurrDataCounter(DMA1_Channel5,USART1_REC_LEN);	//���ô������ݳ���,��װ��,���ý��յ�ַƫַ��0��ʼ
			DMA_Cmd(DMA1_Channel5,ENABLE);  //��DMA����
    } 
		__nop(); 
} 


