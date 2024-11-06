#include "sys.h"
#include "delay.h"
#include "string.h"
#include "usart.h" 
#include "led.h" 		 	 
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"	 
#include "timer.h"
#include "stdlib.h"
#include "ct1711.h"
#include "max30102.h"
#include "algorithm.h"
#include "myiic.h"
//串口数据包
char senddate[16]={0x5A,0xA5,0,0,0,0,0,0,0,0,0,0,0x53,0x54,0x4F,0x50};//缓存容量5A A5 温度2字节（1000倍，高前低后） 心率曲线2字节（高前低后） 心率2字节（高前低后） 心率检测状态  血氧2字节（高前低后） 血氧检测状态 53 54 4F 50
//ct1711参数
float CT1711_temp=0.0;
unsigned int CT1711_hex=0;
//max30102参数
uint32_t aun_ir_buffer[500]; //IR LED sensor data   红外数据，用于计算血氧
int32_t n_ir_buffer_length=500;    //data length  
uint32_t aun_red_buffer[500];    //Red LED sensor data  红光数据，用于计算心率曲线以及计算心率
int32_t n_sp02; //SPO2 value
int8_t ch_spo2_valid;   //indicator to show if the SP02 calculation is valid
int32_t n_heart_rate;   //heart rate value
int8_t  ch_hr_valid;    //indicator to show if the heart rate calculation is valid
#define MAX_BRIGHTNESS 255


int main(void)
{	 
	//USB通信变量
	u32 len;	
	u8 usbstatus=0;		
	//max30102变量
	uint32_t un_min, un_max, un_prev_data;  
	int i,j;
	int32_t n_brightness;
	float f_temp;
	u8 temp[6];

	//初始化系统
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级 
	JTAG_Set(JTAG_SWD_DISABLE);     //=====关闭JTAG接口
  JTAG_Set(SWD_ENABLE);           //=====打开SWD接口 可以利用主板的SWD接口调试
	uart1_init(115200);
	LED_Init();		  		//初始化与LED连接的硬件接口
	TIM3_Int_Init(49,7199);//10Khz的计数频率，计数到50为5ms  
	
	//CT1711初始化
	CT1711_Init();
	
	//max30102初始化
	max30102_init();
	//下面这些在主程序循环执行，在保证硬件没问题的情况下，无需在初始化中执行一次，反正也没有做什么保护和提示措施
//	un_min=0x3FFFF;
//	un_max=0;
//  n_ir_buffer_length=500; //缓冲区长度100存储5秒的运行在100sps的样本
//	//读取前500个样本，确定信号范围
//  for(i=0;i<n_ir_buffer_length;i++){
//    while(MAX30102_INT==1);   //等待直到中断引脚断言
//		max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);//读取传感器数据，赋值到temp中
//		aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];    //  将值合并得到实际数字
//		aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5];   //  将值合并得到实际数字
//		if(un_min>aun_red_buffer[i])
//				un_min=aun_red_buffer[i];//更新计算最小值
//		if(un_max<aun_red_buffer[i])
//				un_max=aun_red_buffer[i];//更新计算最大值
//	}
//	//un_prev_data=aun_red_buffer[i];//获取数据最后一个数值，没有用，主程序中未马上使用被替换
//	//计算前500个样本(前5秒样本)后的心率和SpO2
//	maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 

	
	//USB虚拟串口初始化
	delay_ms(500);
	USB_Port_Set(0); 	//USB先断开
	delay_ms(200);
	USB_Port_Set(1);	//USB再次连接
	Set_USBClock();   
	USB_Interrupts_Config();    
	USB_Init();	
	while(1)
	{
			/**************************USB虚拟串口部分*************************/
			//检测usb热插拔状态
			if(usbstatus!=bDeviceState){//USB连接状态发生了改变.
				usbstatus=bDeviceState;//记录新的状态
//				if(usbstatus==CONFIGURED)printf("USB连接成功\r\n");//提示USB连接成功
//				else printf("USB连接断开\r\n");//提示USB断开
			}
			//终端接收usb虚拟串口数据
			if(USB_USART_RX_STA&0x8000){	
					len=USB_USART_RX_STA&0x3FFF;//得到此次接收到的数据长度
					USB_USART_RX_STA=0;
//					printf("您发送的消息长度为:%d,内容为：%s\r\n",len,USB_USART_RX_BUF);//提示USB接收数据长度
					if(USB_USART_RX_BUF[0]==0x5a&&USB_USART_RX_BUF[1]==0xa5&&USB_USART_RX_BUF[2]==0xff&&USB_USART_RX_BUF[3]==0x50&&len==4){//判断接收数据
							//no deal with...
					}
					memset(USB_USART_RX_BUF,0,sizeof(USB_USART_RX_BUF));//缓存清零			
			}
			if(usbstatus==CONFIGURED){//判断USB已经连接
					//定时500ms进入一次
					if(led_flag>100){//5ms×100=500ms
						led_flag=0;
						//闪烁led代表工作状态正常
						LED0=!LED0;
						//读取温度传感器温度数值
						time5s++;
						if(time5s>=10){
								time5s=0;
							CT1711_temp = CT1711_Read_Temp_Degree();
							CT1711_hex = CT1711_temp*1000;
							//usb_printf("测试温度为:%.8f\r\n",CT1711_temp);	
						}
						
						
					}
					
					
					//读取和计算max30102数据，总体用缓存的500组数据分析，实际每读取100组新数据分析一次
					i=0;
					un_min=0x3FFFF;
					un_max=0;
					//将前100组样本转储到内存中（实际没有），并将后400组样本移到顶部，将100-500缓存数据移位到0-400
					for(i=100;i<500;i++){
							aun_red_buffer[i-100]=aun_red_buffer[i];//将100-500缓存数据移位到0-400
							aun_ir_buffer[i-100]=aun_ir_buffer[i];//将100-500缓存数据移位到0-400
							//更新信号的最小值和最大值
							if(un_min>aun_red_buffer[i])//寻找移位后0-400中的最小值
								un_min=aun_red_buffer[i];
							if(un_max<aun_red_buffer[i])//寻找移位后0-400中的最大值
								un_max=aun_red_buffer[i];
					}
					//在计算心率前取100组样本，取的数据放在400-500缓存数组中
					for(i=400;i<500;i++){
						un_prev_data=aun_red_buffer[i-1];//临时记录上一次读取数据
						while(MAX30102_INT==1);
						max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);//读取传感器数据，赋值到temp中
						aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];  //将值合并得到实际数字，数组400-500为新读取数据
						aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5];   //将值合并得到实际数字，数组400-500为新读取数据
						if(aun_red_buffer[i]>un_prev_data){//用新获取的一个数值与上一个数值对比
							f_temp=aun_red_buffer[i]-un_prev_data;
							f_temp/=(un_max-un_min);
							f_temp*=MAX_BRIGHTNESS;//公式（心率曲线）=（新数值-旧数值）/（最大值-最小值）*255
							n_brightness-=(int)f_temp;
							if(n_brightness<0)
								n_brightness=0;
						}
						else{
							f_temp=un_prev_data-aun_red_buffer[i];
							f_temp/=(un_max-un_min);
							f_temp*=MAX_BRIGHTNESS;//公式（心率曲线）=（旧数值-新数值）/（最大值-最小值）*255
							n_brightness+=(int)f_temp;
							if(n_brightness>MAX_BRIGHTNESS)
								n_brightness=MAX_BRIGHTNESS;
						}
						//通过UART将样本和计算结果发送到终端程序
						//if(ch_hr_valid == 1 && ch_spo2_valid ==1 && n_heart_rate<200 && n_sp02<101){//使用上一次测量的结论作为检测正确性判断//ch_hr_valid == 1 && n_heart_rate<120
  //					usb_printf("HR=%i, ", n_heart_rate); 
	//					usb_printf("HRvalid=%i, ", ch_hr_valid);
	//					usb_printf("SpO2=%i, ", n_sp02);
	//					usb_printf("SPO2Valid=%i\r\n", ch_spo2_valid);
								senddate[2]=CT1711_hex>>8;//温度搞8位
								senddate[3]=CT1711_hex;//温度低8位
								senddate[4]=n_brightness>>8;//心率曲线高8位
								senddate[5]=n_brightness;//心率曲线8位
								senddate[6]=n_heart_rate>>8;//心率高8位
								senddate[7]=n_heart_rate;//心率8位							
								senddate[8]=ch_hr_valid;//心率监测状态
								senddate[9]=n_sp02>>8;//血氧搞8位
								senddate[10]=n_sp02;//血氧低8位
								senddate[11]=ch_spo2_valid;//血氧监测状态						
								for(j=0;j<16;j++){//发送测试完毕标志，提示校验
									USB_USART_SendData(senddate[j]);//以字节方式,发送给USB 
								}		
						//}
					}
					maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);//传入500个心率和血氧数据计算传感器检测结论，反馈心率和血氧测试结果


					
			}
			else{//USB未连接
					//定时50ms进入一次
					if(led_flag>10){//5ms×10=50ms
							led_flag=0;
							LED0=!LED0;
					}
			}
	}
}




