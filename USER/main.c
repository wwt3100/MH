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
#include "MH-Struct.h"
#include "stmflash.h"

//add ������־ ���� �ϴ�
// ���� �ϴ� ����ʽ��

volatile uint32_t g_tick_1ms;

volatile uint32_t LedTimer;

volatile _HostStat hstat;

static void gpio_init(void)
{
    
}

int main(void)
{	 
    u32 total,free;
    FRESULT fres;
    DIR dj;         /* Directory search object */
    FILINFO fno;    /* File information */
	u8 t=0;	
	u8 res=0;	    	    
    SysTick_Init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	gpio_init();
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�

	FLASH_Init();				//��ʼ��W25Q128 
    uart1_init(115200);	 	//���ڳ�ʼ��Ϊ115200
	SD_Init();
    if(SD_CardIsInserted())
    {
        hstat.SDCardStat=1;
        fres=f_mount(fs[0],"0:",1); 					//����SD�� 		
        if(fres!=FR_OK)
            goto startupsderro;
        fres = f_findfirst(&dj, &fno, "", "*.hex");
        if(fres==FR_OK && RCC_GetFlagStatus(RCC_FLAG_SFTRST)==RESET)
        {
            UpDataViaSDCard(&fno);
        }
        exfuns_init();							//Ϊfatfs��ر��������ڴ�        
        exf_getfree("0",&total,&free);	//�õ�SD������������ʣ������
	}
    else    
    {
        startupsderro:
        hstat.SDCardStat=0;
    }
	while(1)
	{
		LED3(0);
		Delay_ms(500);	
        LED3(1);   
        Delay_ms(500);        
	} 
}
















