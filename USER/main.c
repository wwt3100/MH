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
	//delay_init();	    	 //延时函数初始化	  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart1_init(115200);	 	//串口初始化为115200
 	LED_Init();		  			//初始化与LED连接的硬件接口

	FLASH_Init();				//初始化W25Q128 
	SD_Init();
 	exfuns_init();							//为fatfs相关变量申请内存				 
  	f_mount(fs[0],"0:",1); 					//挂载SD卡 												    
	while(exf_getfree("0",&total,&free))	//得到SD卡的总容量和剩余容量
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
















