#include "sys.h"
#include "usart.h"	 

////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 







u8 USART1_RX_BUF[USART1_REC_LEN];     //最大USART_REC_LEN个字节.
u8 USART1_TX_BUF[USART1_REC_LEN];     //最大USART_REC_LEN个字节.
u16 Usart1_TX_Cou;             //保存DMA每次数据发送的长度
u16 Usart1_RX_Cou = 0;
u8 Usart1_RX_Flag = 0;
//初始化IO 串口1 
//bound:波特率
void uart1_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  USART_DeInit(USART1);  //复位串口1

	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
	USART_ITConfig(USART1,USART_IT_TC,DISABLE); 		//关闭发送完成中断
	USART_ITConfig(USART1,USART_IT_RXNE,DISABLE);		//关闭接收完成中断
	USART_ITConfig(USART1,USART_IT_IDLE,ENABLE);    //开启接收空闲中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 
}
//如要要更改为其他串口发送要更改下面带*号的
//DMA1的各通道配置
//这里的传输形式是固定的,这点要根据不同的情况来修改
//从存储器->外设模式/8位数据宽度/存储器增量模式
//cmar:存储器地址，例如定义u8 Usart1_TX[10];这里填写(u32)Usart1_TX， 如果发送单个变量需要用(u32)&i，如果i占2字节 那么cndtr就要填2
//cndtr:数据传输量 
void USART1_DMA_TX(u32 cmar, u16 cndtr)
{
 	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA时钟
	
	DMA_DeInit(DMA1_Channel4);   //将DMA的通道1寄存器重设为缺省值*
	Usart1_TX_Cou=cndtr;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;  //DMA外设ADC基地址,*
	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //数据传输方向，从内存读取发送到外设
	DMA_InitStructure.DMA_BufferSize = cndtr;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器*
	
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //使能串口1的DMA发送*
	DMA_Cmd(DMA1_Channel4, DISABLE );  //关闭USART1 TX DMA1 所指示的通道*
	DMA_SetCurrDataCounter(DMA1_Channel4,cndtr);//DMA通道的DMA缓存的大小*
	DMA_Cmd(DMA1_Channel4, ENABLE);  //使能USART1 TX DMA1 所指示的通道 *
} 
void USART1_DMA_RX(void)
{
	//串口收DMA配置
	DMA_InitTypeDef DMA_InitStructure;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//启动DMA时钟
	
	DMA_DeInit(DMA1_Channel5);   //DMA1通道5配置
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);   //外设地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART1_RX_BUF;   //内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;        //dma传输方向单向
	DMA_InitStructure.DMA_BufferSize = USART1_REC_LEN;      
	//设置DMA在传输时缓冲区的长度，这个值取预计接收的最大量，
	//例如Usart1_RX_LEN为512，当接收到一个字节后，用u32 i=DMA_GetCurrDataCounter(DMA1_Channel5); 这是i的值就是511，
	//那么用u32 Cou = Usart1_RX_LEN - DMA_GetCurrDataCounter(DMA1_Channel5);就得到了接收字节的计数，那么Cou = 1;
	
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 		//设置DMA的外设递增模式，一个外设
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  	 //设置DMA的内存递增模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//外设数据字长
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 	//内存数据字长
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  				//设置DMA的传输模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; //设置DMA的优先级别
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;			 //设置DMA的2个memory中的变量互相访问
	DMA_Init(DMA1_Channel5,&DMA_InitStructure);

	DMA_Cmd(DMA1_Channel5,ENABLE);	//使能通道5
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);		//采用DMA方式接收
}
//串口1空闲帧中断   
void USART1_IRQHandler(void)                               
{   
	uint32_t temp = 0;
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  //判断是否真的空闲帧来了
    {
    	//USART_ClearFlag(USART1,USART_IT_IDLE);
    	temp = USART1->SR;	//先读SR，然后读DR才能清除
    	temp = USART1->DR; //清USART_IT_IDLE标志
    	DMA_Cmd(DMA1_Channel5,DISABLE);   //关闭DMA接收，防止其间还有数据过来
			temp = USART1_REC_LEN - DMA_GetCurrDataCounter(DMA1_Channel5); //计算接收的字节数
			Usart1_RX_Cou = temp;    //保存接收到的字节数,main中处理后要清零
			Usart1_RX_Flag = 1;				//串口1一个数据包接收完成，标志置一，main中处理后要清零
			DMA_SetCurrDataCounter(DMA1_Channel5,USART1_REC_LEN);	//设置传输数据长度,重装填,并让接收地址偏址从0开始
			DMA_Cmd(DMA1_Channel5,ENABLE);  //打开DMA接收
    } 
		__nop(); 
} 


