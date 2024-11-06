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
//�������ݰ�
char senddate[16]={0x5A,0xA5,0,0,0,0,0,0,0,0,0,0,0x53,0x54,0x4F,0x50};//��������5A A5 �¶�2�ֽڣ�1000������ǰ�ͺ� ��������2�ֽڣ���ǰ�ͺ� ����2�ֽڣ���ǰ�ͺ� ���ʼ��״̬  Ѫ��2�ֽڣ���ǰ�ͺ� Ѫ�����״̬ 53 54 4F 50
//ct1711����
float CT1711_temp=0.0;
unsigned int CT1711_hex=0;
//max30102����
uint32_t aun_ir_buffer[500]; //IR LED sensor data   �������ݣ����ڼ���Ѫ��
int32_t n_ir_buffer_length=500;    //data length  
uint32_t aun_red_buffer[500];    //Red LED sensor data  ������ݣ����ڼ������������Լ���������
int32_t n_sp02; //SPO2 value
int8_t ch_spo2_valid;   //indicator to show if the SP02 calculation is valid
int32_t n_heart_rate;   //heart rate value
int8_t  ch_hr_valid;    //indicator to show if the heart rate calculation is valid
#define MAX_BRIGHTNESS 255


int main(void)
{	 
	//USBͨ�ű���
	u32 len;	
	u8 usbstatus=0;		
	//max30102����
	uint32_t un_min, un_max, un_prev_data;  
	int i,j;
	int32_t n_brightness;
	float f_temp;
	u8 temp[6];

	//��ʼ��ϵͳ
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ� 
	JTAG_Set(JTAG_SWD_DISABLE);     //=====�ر�JTAG�ӿ�
  JTAG_Set(SWD_ENABLE);           //=====��SWD�ӿ� �������������SWD�ӿڵ���
	uart1_init(115200);
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�
	TIM3_Int_Init(49,7199);//10Khz�ļ���Ƶ�ʣ�������50Ϊ5ms  
	
	//CT1711��ʼ��
	CT1711_Init();
	
	//max30102��ʼ��
	max30102_init();
	//������Щ��������ѭ��ִ�У��ڱ�֤Ӳ��û���������£������ڳ�ʼ����ִ��һ�Σ�����Ҳû����ʲô��������ʾ��ʩ
//	un_min=0x3FFFF;
//	un_max=0;
//  n_ir_buffer_length=500; //����������100�洢5���������100sps������
//	//��ȡǰ500��������ȷ���źŷ�Χ
//  for(i=0;i<n_ir_buffer_length;i++){
//    while(MAX30102_INT==1);   //�ȴ�ֱ���ж����Ŷ���
//		max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);//��ȡ���������ݣ���ֵ��temp��
//		aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];    //  ��ֵ�ϲ��õ�ʵ������
//		aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5];   //  ��ֵ�ϲ��õ�ʵ������
//		if(un_min>aun_red_buffer[i])
//				un_min=aun_red_buffer[i];//���¼�����Сֵ
//		if(un_max<aun_red_buffer[i])
//				un_max=aun_red_buffer[i];//���¼������ֵ
//	}
//	//un_prev_data=aun_red_buffer[i];//��ȡ�������һ����ֵ��û���ã���������δ����ʹ�ñ��滻
//	//����ǰ500������(ǰ5������)������ʺ�SpO2
//	maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 

	
	//USB���⴮�ڳ�ʼ��
	delay_ms(500);
	USB_Port_Set(0); 	//USB�ȶϿ�
	delay_ms(200);
	USB_Port_Set(1);	//USB�ٴ�����
	Set_USBClock();   
	USB_Interrupts_Config();    
	USB_Init();	
	while(1)
	{
			/**************************USB���⴮�ڲ���*************************/
			//���usb�Ȳ��״̬
			if(usbstatus!=bDeviceState){//USB����״̬�����˸ı�.
				usbstatus=bDeviceState;//��¼�µ�״̬
//				if(usbstatus==CONFIGURED)printf("USB���ӳɹ�\r\n");//��ʾUSB���ӳɹ�
//				else printf("USB���ӶϿ�\r\n");//��ʾUSB�Ͽ�
			}
			//�ն˽���usb���⴮������
			if(USB_USART_RX_STA&0x8000){	
					len=USB_USART_RX_STA&0x3FFF;//�õ��˴ν��յ������ݳ���
					USB_USART_RX_STA=0;
//					printf("�����͵���Ϣ����Ϊ:%d,����Ϊ��%s\r\n",len,USB_USART_RX_BUF);//��ʾUSB�������ݳ���
					if(USB_USART_RX_BUF[0]==0x5a&&USB_USART_RX_BUF[1]==0xa5&&USB_USART_RX_BUF[2]==0xff&&USB_USART_RX_BUF[3]==0x50&&len==4){//�жϽ�������
							//no deal with...
					}
					memset(USB_USART_RX_BUF,0,sizeof(USB_USART_RX_BUF));//��������			
			}
			if(usbstatus==CONFIGURED){//�ж�USB�Ѿ�����
					//��ʱ500ms����һ��
					if(led_flag>100){//5ms��100=500ms
						led_flag=0;
						//��˸led������״̬����
						LED0=!LED0;
						//��ȡ�¶ȴ������¶���ֵ
						time5s++;
						if(time5s>=10){
								time5s=0;
							CT1711_temp = CT1711_Read_Temp_Degree();
							CT1711_hex = CT1711_temp*1000;
							//usb_printf("�����¶�Ϊ:%.8f\r\n",CT1711_temp);	
						}
						
						
					}
					
					
					//��ȡ�ͼ���max30102���ݣ������û����500�����ݷ�����ʵ��ÿ��ȡ100�������ݷ���һ��
					i=0;
					un_min=0x3FFFF;
					un_max=0;
					//��ǰ100������ת�����ڴ��У�ʵ��û�У���������400�������Ƶ���������100-500����������λ��0-400
					for(i=100;i<500;i++){
							aun_red_buffer[i-100]=aun_red_buffer[i];//��100-500����������λ��0-400
							aun_ir_buffer[i-100]=aun_ir_buffer[i];//��100-500����������λ��0-400
							//�����źŵ���Сֵ�����ֵ
							if(un_min>aun_red_buffer[i])//Ѱ����λ��0-400�е���Сֵ
								un_min=aun_red_buffer[i];
							if(un_max<aun_red_buffer[i])//Ѱ����λ��0-400�е����ֵ
								un_max=aun_red_buffer[i];
					}
					//�ڼ�������ǰȡ100��������ȡ�����ݷ���400-500����������
					for(i=400;i<500;i++){
						un_prev_data=aun_red_buffer[i-1];//��ʱ��¼��һ�ζ�ȡ����
						while(MAX30102_INT==1);
						max30102_FIFO_ReadBytes(REG_FIFO_DATA,temp);//��ȡ���������ݣ���ֵ��temp��
						aun_red_buffer[i] =  (long)((long)((long)temp[0]&0x03)<<16) | (long)temp[1]<<8 | (long)temp[2];  //��ֵ�ϲ��õ�ʵ�����֣�����400-500Ϊ�¶�ȡ����
						aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03)<<16) |(long)temp[4]<<8 | (long)temp[5];   //��ֵ�ϲ��õ�ʵ�����֣�����400-500Ϊ�¶�ȡ����
						if(aun_red_buffer[i]>un_prev_data){//���»�ȡ��һ����ֵ����һ����ֵ�Ա�
							f_temp=aun_red_buffer[i]-un_prev_data;
							f_temp/=(un_max-un_min);
							f_temp*=MAX_BRIGHTNESS;//��ʽ���������ߣ�=������ֵ-����ֵ��/�����ֵ-��Сֵ��*255
							n_brightness-=(int)f_temp;
							if(n_brightness<0)
								n_brightness=0;
						}
						else{
							f_temp=un_prev_data-aun_red_buffer[i];
							f_temp/=(un_max-un_min);
							f_temp*=MAX_BRIGHTNESS;//��ʽ���������ߣ�=������ֵ-����ֵ��/�����ֵ-��Сֵ��*255
							n_brightness+=(int)f_temp;
							if(n_brightness>MAX_BRIGHTNESS)
								n_brightness=MAX_BRIGHTNESS;
						}
						//ͨ��UART�������ͼ��������͵��ն˳���
						//if(ch_hr_valid == 1 && ch_spo2_valid ==1 && n_heart_rate<200 && n_sp02<101){//ʹ����һ�β����Ľ�����Ϊ�����ȷ���ж�//ch_hr_valid == 1 && n_heart_rate<120
  //					usb_printf("HR=%i, ", n_heart_rate); 
	//					usb_printf("HRvalid=%i, ", ch_hr_valid);
	//					usb_printf("SpO2=%i, ", n_sp02);
	//					usb_printf("SPO2Valid=%i\r\n", ch_spo2_valid);
								senddate[2]=CT1711_hex>>8;//�¶ȸ�8λ
								senddate[3]=CT1711_hex;//�¶ȵ�8λ
								senddate[4]=n_brightness>>8;//�������߸�8λ
								senddate[5]=n_brightness;//��������8λ
								senddate[6]=n_heart_rate>>8;//���ʸ�8λ
								senddate[7]=n_heart_rate;//����8λ							
								senddate[8]=ch_hr_valid;//���ʼ��״̬
								senddate[9]=n_sp02>>8;//Ѫ����8λ
								senddate[10]=n_sp02;//Ѫ����8λ
								senddate[11]=ch_spo2_valid;//Ѫ�����״̬						
								for(j=0;j<16;j++){//���Ͳ�����ϱ�־����ʾУ��
									USB_USART_SendData(senddate[j]);//���ֽڷ�ʽ,���͸�USB 
								}		
						//}
					}
					maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);//����500�����ʺ�Ѫ�����ݼ��㴫���������ۣ��������ʺ�Ѫ�����Խ��


					
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




