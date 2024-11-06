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

char senddate[100];//��������
float CT1711_temp=0.0;
 
 
uint32_t aun_ir_buffer[500]; //IR LED sensor data
int32_t n_ir_buffer_length;    //data length
uint32_t aun_red_buffer[500];    //Red LED sensor data
int32_t n_sp02; //SPO2 value
int8_t ch_spo2_valid;   //indicator to show if the SP02 calculation is valid
int32_t n_heart_rate;   //heart rate value
int8_t  ch_hr_valid;    //indicator to show if the heart rate calculation is valid
uint8_t uch_dummy;
#define MAX_BRIGHTNESS 255


int main(void)
{	 
	u32 len;	
	u8 usbstatus=0;		

	
	uint32_t un_min, un_max, un_prev_data;  
	int i;
	int32_t n_brightness;
	float f_temp;
	u8 temp_num=0;
	u8 temp[6];
	u8 str[100];
	u8 dis_hr=0,dis_spo2=0;

	
	
	//��ʼ��ϵͳ
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ� 
	JTAG_Set(JTAG_SWD_DISABLE);     //=====�ر�JTAG�ӿ�
  JTAG_Set(SWD_ENABLE);           //=====��SWD�ӿ� �������������SWD�ӿڵ���
	uart1_init(115200);
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�
	TIM3_Int_Init(49,7199);//10Khz�ļ���Ƶ�ʣ�������50Ϊ5ms  
	CT1711_Init();//CT1711��ʼ��
	max30102_init();
	un_min=0x3FFFF;
	un_max=0;
	
	  n_ir_buffer_length=500; //buffer length of 100 stores 5 seconds of samples running at 100sps
	//read the first 500 samples, and determine the signal range
    for(i=0;i<n_ir_buffer_length;i++)
    {
        while(MAX30102_INT==1);   //wait until the interrupt pin asserts
        
		max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
		aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];    // Combine values to get the actual number
		aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5];   // Combine values to get the actual number
            
        if(un_min>aun_red_buffer[i])
            un_min=aun_red_buffer[i];    //update signal min
        if(un_max<aun_red_buffer[i])
            un_max=aun_red_buffer[i];    //update signal max
    }
	  un_prev_data=aun_red_buffer[i];
	//calculate heart rate and SpO2 after first 500 samples (first 5 seconds of samples)
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 

	
	//USB���⴮�ڳ�ʼ��
	delay_ms(500);
	USB_Port_Set(0); 	//USB�ȶϿ�
	delay_ms(200);
	USB_Port_Set(1);	//USB�ٴ�����
	Set_USBClock();   
	USB_Interrupts_Config();    
	USB_Init();	
	memset(senddate,0,sizeof(senddate));//��������	
	while(1)
	{
			/**************************USB���⴮�ڲ���*************************/
			//���usb�Ȳ��״̬
			if(usbstatus!=bDeviceState){//USB����״̬�����˸ı�.
				usbstatus=bDeviceState;//��¼�µ�״̬
				if(usbstatus==CONFIGURED)printf("USB���ӳɹ�\r\n");//��ʾUSB���ӳɹ�
				else printf("USB���ӶϿ�\r\n");//��ʾUSB�Ͽ�
			}
			//�ն˽���usb���⴮������
			if(USB_USART_RX_STA&0x8000){	
					len=USB_USART_RX_STA&0x3FFF;//�õ��˴ν��յ������ݳ���
					printf("�����͵���Ϣ����Ϊ:%d,����Ϊ��%s\r\n",len,USB_USART_RX_BUF);//��ʾUSB�������ݳ���
					USB_USART_RX_STA=0;
					if(USB_USART_RX_BUF[0]==0x5a&&USB_USART_RX_BUF[1]==0xa5&&USB_USART_RX_BUF[2]==0xff&&USB_USART_RX_BUF[3]==0x50&&len==4){//���������ڴ�
							//no deal with...
					}
					memset(USB_USART_RX_BUF,0,len);//��������			
			}
			if(usbstatus==CONFIGURED){//�ж�USB�Ѿ�����
					//��ʱ500ms����һ��
					if(led_flag>100){//5ms��100=500ms
						led_flag=0;
						LED0=!LED0;
//						for(int i=0;i<20;i++){//���Ͳ�����ϱ�־����ʾУ��
//							senddate[i]=0xAA;//��ֵ����AA
//						  senddate[19]=0xBB;
//							USB_USART_SendData(senddate[i]);//���ֽڷ�ʽ,���͸�USB 
//						}	
						CT1711_temp = CT1711_Read_Temp_Degree();
						//usb_printf("�����¶�Ϊ:%.8f\r\n",CT1711_temp);
					
					}
					
					
					
				i=0;
        un_min=0x3FFFF;
        un_max=0;
		
		//dumping the first 100 sets of samples in the memory and shift the last 400 sets of samples to the top
        for(i=100;i<500;i++)
        {
            aun_red_buffer[i-100]=aun_red_buffer[i];
            aun_ir_buffer[i-100]=aun_ir_buffer[i];
            
            //update the signal min and max
            if(un_min>aun_red_buffer[i])
            un_min=aun_red_buffer[i];
            if(un_max<aun_red_buffer[i])
            un_max=aun_red_buffer[i];
        }
		//take 100 sets of samples before calculating the heart rate.
        for(i=400;i<500;i++)
        {
            un_prev_data=aun_red_buffer[i-1];
            while(MAX30102_INT==1);
            max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);
			aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];    // Combine values to get the actual number
			aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5];   // Combine values to get the actual number
        
            if(aun_red_buffer[i]>un_prev_data)
            {
                f_temp=aun_red_buffer[i]-un_prev_data;
                f_temp/=(un_max-un_min);
                f_temp*=MAX_BRIGHTNESS;
                n_brightness-=(int)f_temp;
                if(n_brightness<0)
                    n_brightness=0;
            }
            else
            {
                f_temp=un_prev_data-aun_red_buffer[i];
                f_temp/=(un_max-un_min);
                f_temp*=MAX_BRIGHTNESS;
                n_brightness+=(int)f_temp;
                if(n_brightness>MAX_BRIGHTNESS)
                    n_brightness=MAX_BRIGHTNESS;
            }
			//send samples and calculation result to terminal program through UART
			if(ch_hr_valid == 1 && n_heart_rate<120)//**/ ch_hr_valid == 1 && ch_spo2_valid ==1 && n_heart_rate<120 && n_sp02<101
			{
				dis_hr = n_heart_rate;
				dis_spo2 = n_sp02;
			}
			else
			{
				dis_hr = 0;
				dis_spo2 = 0;
			}
				usb_printf("HR=%i, ", n_heart_rate); 
				usb_printf("HRvalid=%i, ", ch_hr_valid);
				usb_printf("SpO2=%i, ", n_sp02);
				usb_printf("SPO2Valid=%i\r\n", ch_spo2_valid);

		}
     maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);

					
					
					
					
					
					

					
					
					
			}
			else{//USBδ����
					//��ʱ50ms����һ��
					if(led_flag>10){//5ms��10=50ms
							led_flag=0;
							LED0=!LED0;
					}
			}
	}
}




