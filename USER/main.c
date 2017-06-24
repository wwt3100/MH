#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	  
#include "usmart.h" 
#include "sdio_sdcard.h"  
#include "SPI_Flash.h"    
#include "ff.h"  
#include "exfuns.h"    
#include "SysTick.h"


//add ������־ ���� �ϴ�
// ���� �ϴ� ����ʽ��

volatile uint32_t g_tick_1ms;

volatile uint32_t LedTimer;

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
	exf_getfree("0",&total,&free);	//�õ�SD������������ʣ������
													  			    
	while(1)
	{
		LED3(0);
		Delay_ms(500);	
        LED3(1);   
        Delay_ms(500);        
	} 
}
















