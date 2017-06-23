#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	  
#include "usmart.h" 
#include "sdio_sdcard.h"  
#include "spi.h"    
#include "ff.h"  
#include "exfuns.h"    
#include "SysTick.h"

uint32_t g_tick_1ms;

 int main(void)
 {	 
 	u32 total,free;
	u8 t=0;	
	u8 res=0;	    	    
    SysTick_Init();
	//delay_init();	    	 //��ʱ������ʼ��	  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart1_init(115200);	 	//���ڳ�ʼ��Ϊ115200
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�

	FLASH_Init();				//��ʼ��W25Q128 
	SD_Init();
 	exfuns_init();							//Ϊfatfs��ر��������ڴ�				 
  	f_mount(fs[0],"0:",1); 					//����SD�� 												    
	while(exf_getfree("0",&total,&free))	//�õ�SD������������ʣ������
	{
		
		Delay_ms(200);
		Delay_ms(200);
	}													  			    
	while(1)
	{
		t++; 
		Delay_ms(200);		 			   
	} 
}
















