#include "sys.h"
//#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	  
//#include "usmart.h" 
#include "sdio_sdcard.h"  
#include "SPI_Flash.h"    
#include "ff.h"  
//#include "exfuns.h"    
#include "SysTick.h"
#include "MH-Struct.h"
#include "stmflash.h"
#include "mbuf.h"
#include "rtc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "485Server.h"

//add 报警日志 保存 上传
// 卡坏 上传 不格式化

volatile uint32_t g_tick_1ms;

uint32_t Led1Timer=0,Led2Timer=0,Led3Timer=0;

volatile _HostStat hstat;
volatile _GlobalConfig _gc;
FATFS *fs;

const _DeviceConfig cDc[255]__attribute__((at(0x08008000)))={0};

extern __mbuf *u1mbuf,*u2mbuf,*u3mbuf,*gmbuf;
extern struct rtc_time systmtime;

static void gpio_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
 	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	 //使能PB端口时钟
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOD, &GPIO_InitStructure);					 //根据设定参数初始化GPIOD.8
    GPIO_ResetBits(GPIOD,GPIO_Pin_8);						//485CTR

}

int main(void)
{	 
    uint8_t l=0;
    unsigned long fre_clust,free;//,total;
    FRESULT fres;
    _DeviceConfig dc;
    strcpy((char*)dc.ID,"HS300BS58F");
    _gc.MonitorDeviceNum=1;
    _gc.SamplingInterval=10;
    _gc.RetryInterval=11;
    
    
    fs=(FATFS*)malloc(sizeof(FATFS));	    
    SysTick_Init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	gpio_init();
 	LED_Init();		  			//初始化与LED连接的硬件接口
    RTC_Init();
	FLASH_Init();				//初始化W25Q128 
    USART1_Init(115200);	 	//串口初始化为115200
    USART3_Init(19200);
	SD_Init();
    if(SD_CardIsInserted())
    {
        
        fres=f_mount(fs,"0:",1); 					//挂载SD卡 		
        if(fres!=FR_OK)
            goto startupsderro;
        fres=f_getfree("0:",&fre_clust,&fs); 					//检测剩余空间
        if(fres==FR_OK)
        {
            //total = (fs->n_fatent - 2) * fs->csize;
            free = fre_clust * fs->csize;
            if(free<8192)      
                hstat.SDCardStat=3;         //SD卡空间不足
            else 
                hstat.SDCardStat=1;
        }
        else
        {
            hstat.SDCardStat=2;
        }
	}
    else    
    {
        startupsderro:
        hstat.SDCardStat=0;
    }
    timer_init(&Led3Timer,500);
	while(1)
	{
        Server_Process();
        if(timer_check(Led3Timer))
        {
            timer_init(&Led3Timer,2000);
            l=~l;
            LED3(l);
        }
        
	} 
}
















