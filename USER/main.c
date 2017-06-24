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

//add 报警日志 保存 上传
// 卡坏 上传 不格式化

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
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	gpio_init();
 	LED_Init();		  			//初始化与LED连接的硬件接口

	FLASH_Init();				//初始化W25Q128 
    uart1_init(115200);	 	//串口初始化为115200
	SD_Init();
    if(SD_CardIsInserted())
    {
        hstat.SDCardStat=1;
        fres=f_mount(fs[0],"0:",1); 					//挂载SD卡 		
        if(fres!=FR_OK)
            goto startupsderro;
        fres = f_findfirst(&dj, &fno, "", "*.hex");
        if(fres==FR_OK && RCC_GetFlagStatus(RCC_FLAG_SFTRST)==RESET)
        {
            UpDataViaSDCard(&fno);
        }
        exfuns_init();							//为fatfs相关变量申请内存        
        exf_getfree("0",&total,&free);	//得到SD卡的总容量和剩余容量
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
















