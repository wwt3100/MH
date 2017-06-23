#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	  
#include "usmart.h" 
#include "sdio_sdcard.h"  
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"    



 int main(void)
 {	 
 	u32 total,free;
	u8 t=0;	
	u8 res=0;	    	    

	delay_init();	    	 //��ʱ������ʼ��	  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
	usmart_dev.init(72);		//��ʼ��USMART		
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�

	W25QXX_Init();				//��ʼ��W25Q128 
	while(SD_Init())//��ⲻ��SD��
	{
	}
 	exfuns_init();							//Ϊfatfs��ر��������ڴ�				 
  	f_mount(fs[0],"0:",1); 					//����SD�� 
 	res=f_mount(fs[1],"1:",1); 				//����FLASH.	
	if(res==0X0D)//FLASH����,FAT�ļ�ϵͳ����,���¸�ʽ��FLASH
	{
		res=f_mkfs("1:",1,4096);//��ʽ��FLASH,1,�̷�;1,����Ҫ������,8������Ϊ1����
		if(res==0)
		{
			f_setlabel((const TCHAR *)"1:HUATO");	//����Flash���̵�����Ϊ��ALIENTEK
			
		}
        else
            ;	//��ʽ��ʧ��
		delay_ms(1000);
	}													    
	while(exf_getfree("0",&total,&free))	//�õ�SD������������ʣ������
	{
		
		delay_ms(200);
		delay_ms(200);
	}													  			    
	while(1)
	{
		t++; 
		delay_ms(200);		 			   
	} 
}
















