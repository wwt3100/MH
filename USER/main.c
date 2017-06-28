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

//add ������־ ���� �ϴ�
// ���� �ϴ� ����ʽ��

volatile uint32_t g_tick_1ms;

uint32_t Led1Timer=0,Led2Timer=0,Led3Timer=0;

volatile _HostStat hstat;
volatile _GlobalConfig _gc;


const _DeviceConfig cDc[255]__attribute__((at(0x08008000)))={0};
const char MHID[12]={"MH6001A001"};//__attribute__((at(0x08008000)))={"MH6001A001"};
_DeviceData _Dd[255]={0};
extern __mbuf *u1mbuf,*u2mbuf,*u3mbuf,*gmbuf;
extern struct rtc_time systmtime;

static void gpio_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
 	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	 //ʹ��PB�˿�ʱ��
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
    GPIO_Init(GPIOD, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOD.8
    GPIO_ResetBits(GPIOD,GPIO_Pin_8);						//485CTR

}

int main(void)
{	 
    uint8_t l=0;
    unsigned long fre_clust,freespace;//,total;
    FRESULT fres;
    FATFS *fs;
    _DeviceConfig dc;
    strcpy((char*)dc.ID,"HS500BS657");
    _gc.MonitorDeviceNum=1;
    _gc.SamplingInterval=10;
    _gc.RetryInterval=11;
    
    
    fs=malloc(sizeof(FATFS));	    
    SysTick_Init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	gpio_init();
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
    RTC_Init();
	FLASH_Init();				//��ʼ��W25Q128 
    USART1_Init(115200);	 	//���ڳ�ʼ��Ϊ115200
    USART3_Init(19200);
	SD_Init();
    if(SD_CardIsInserted())
    {
        
        fres=f_mount(fs,"0:",1); 					//����SD�� 		
        if(fres!=FR_OK)
            goto startupsderro;
        fres=f_getfree("0:",&fre_clust,&fs); 					//���ʣ��ռ�
        if(fres==FR_OK)
        {
            //total = (fs->n_fatent - 2) * fs->csize;
            freespace = fre_clust * fs->csize;
            if(freespace<8192)      
                hstat.SDCardStat=3;         //SD���ռ䲻��
            else 
                hstat.SDCardStat=1;
        }
        else
        {
            hstat.SDCardStat=2;
        }
        f_mount(0,"0:",1);
	}
    else    
    {
        startupsderro:
        hstat.SDCardStat=0;
    }
    free(fs);
    timer_init(&Led3Timer,500);
	while(1)
	{
        Server_Process();
        if(timer_check(Led3Timer))
        {
            timer_init(&Led3Timer,2000);
            (l==0)?(l=1):(l=0);
            LED3(l);
        }
        
	} 
}
















