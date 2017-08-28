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
#include "elr_mpl.h"
#include "rtc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "485Server.h"
#include "Client.h"
#include "SMSAlarm.h"
//add 报警日志 保存 上传
// 卡坏 上传 不格式化

volatile uint32_t g_tick_1ms;

uint32_t Led1Timer=0,Led1Timer2=0,Led2Timer=0,Led2Timer2=0,Led3Timer=0;
//uint32_t SMSAlarmTimer=1;
volatile _HostStat hstat;
volatile _GlobalConfig _gc;
const _GlobalConfig c_gc __attribute__((at(0x08038000)))={
    .AlarmIntervalTime=1,
    .OverLimitInterval=1,
    .OfflineAlarmInterval=1,
    .OverLimitONOFF=1,
    .AlarmONOFF=1,
    .OfflineAlarmONOFF=1,
    .SMSAlarmNum=0,
    .SamplingInterval=1,
};

FATFS *fs;
const _HardwareInfo _hi __attribute__((at(0x08001000)))={0x80000000,0xC000000,0xFEDCBA98};
const _DeviceConfig cDc[255] __attribute__((at(0x08040000)))={0};
const char MHID[] __attribute__((at(0x08039000)))={"MH6001A209"};
_DeviceData _Dd[255]={0};
extern uint8_t *SMSAlarmMessage;
extern __mbuf *u1mbuf,*u2mbuf,*u3mbuf,*gmbuf;
extern __abuf *abuf;

extern uint16_t GSMWorkStat;

extern struct rtc_time systmtime;

extern uint8_t NotConfiging;  //不在配置中
extern uint8_t stat;

static void gpio_init(void)
{
//    EXTI_InitTypeDef EXTI_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;
//    NVIC_InitTypeDef NVIC_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	 //使能PB端口时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOD, &GPIO_InitStructure);					 //根据设定参数初始化GPIOD.8
    GPIO_ResetBits(GPIOD,GPIO_Pin_8);						//485CTR
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOA, &GPIO_InitStructure);	
    GPIO_ResetBits(GPIOA,GPIO_Pin_4);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOD.8
    GPIO_ResetBits(GPIOC,GPIO_Pin_6);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;  //断电中断
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);	
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;  //Line脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; //RJ45其他未用脚
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
    
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource7);
//    EXTI_InitStructure.EXTI_Line=EXTI_Line7;
//    EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
//    EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Rising;
//    EXTI_InitStructure.EXTI_LineCmd=ENABLE;
//    EXTI_Init(&EXTI_InitStructure);
    
//    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn; //
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
}
//FIL *logfile;
uint32_t PowerDownTimer=0;
uint32_t StartUptimer=0;
int main(void)
{	 
    uint8_t l=0,k=0,j=0;
//    unsigned long long fre_clust,freespace;//,total;
//    FRESULT fres=FR_INVALID_DRIVE;
    RCC_LSICmd(ENABLE);
    abuf=CreateAlarmbuf(124);
    memcpy((uint8_t*)&_gc,(uint8_t*)&c_gc,sizeof(_GlobalConfig));
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)==RESET);
    //_gc.RetryInterval=11;
    #ifdef _DEBUG
        DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE);  
    #endif
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_32);
    IWDG_SetReload(0xFFF); //2s 左右
    IWDG_ReloadCounter();
    IWDG_Enable();
    if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST)==SET) //狗复位不提示
    {
        timer_init(&StartUptimer,60000);
    }
    fs=malloc(1020);	    
    SysTick_Init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	gpio_init();
 	LED_Init();		  			//初始化与LED连接的硬件接口
    RTC_Init();
	FLASH_Init();				//初始化W25Q128 
    USART1_Init(115200);	 	//串口初始化为115200
    USART2_Init(9600);	 	//串口初始化为115200
    USART3_Init(19200);
	SD_Init();
    //logfile=malloc(sizeof(FIL));
    //f_mount(fs,"0:",0);
//    if(SD_CardIsInserted())
//    {
//        
//        fres=f_mount(fs,"0:",0); 					//挂载SD卡 		
//        if(fres!=FR_OK)
//            goto startupsderro;
//        fres=f_getfree("0:",&fre_clust,&fs); 					//检测剩余空间
//        if(fres==FR_OK)
//        {
//            //total = (fs->n_fatent - 2) * fs->csize;
//            freespace = fre_clust * fs->csize;
//            if(freespace<8192)      
//                hstat.SDCardStat=20;         //SD卡空间不足
//            else 
//                hstat.SDCardStat=fres;
//        }
//        else
//        {
//            hstat.SDCardStat=fres;
//        }
////        f_mount(0,"0:",1);
//	}
//    else    
//    {
//        startupsderro:
//        hstat.SDCardStat=fres;
//    }
    //timer_init(&Led3Timer,500);
    LED2(Bit_SET);
    //f_open(logfile,"LogFile.log",FA_OPEN_APPEND | FA_WRITE);
    
	while(1)
	{
        //IWDG_ReloadCounter();   //喂狗
        if(timer_check(StartUptimer) && StartUptimer!=0)//|| ( GSMWorkStat==eGSMStat_Ready&&StartUptimer!=0))
        {
            StartUptimer=0;
            SMSAlarm(eAlarmType_PowerOn,0,0);
        }
        if(GSMWorkStat==eGSMStat_Ready)
        {
            if(timer_check_nolimit(Led2Timer))  //设备心跳灯
            {
                IWDG_ReloadCounter();  //喂狗
                if(timer_check_nolimit(Led2Timer2))
                {
                    timer_init(&Led2Timer,1500);
                    timer_init(&Led2Timer2,1550);
                }
                LED2(Bit_RESET);
            }
            else
            {
                LED2(Bit_SET);
            }
        }
        else
        {
            if(timer_check_nolimit(Led2Timer))
            {
                IWDG_ReloadCounter();  //喂狗
                timer_init(&Led2Timer,1000);
                (k==0)?(k=1):(k=0);
                LED2(k);
            }
        }
        if((hstat.SDCardStat!=FR_OK ||GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7)==Bit_SET) && Led1Timer==0)  //SD卡异常报警
        {
            timer_init(&Led1Timer,1000);
        }
        if(hstat.SDCardStat==FR_OK && GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7)==Bit_RESET)
        {
            Led1Timer=0;
            
            if(stat==e_Stat_SampleingWait)// || stat==e_Stat_Sampling)
            {
                //LED1(Bit_SET);
                if(timer_check_nolimit(Led1Timer2))
                {
                    (j==0)?(j=1):(j=0);
                    timer_init(&Led1Timer2,100);
                    LED1(j);
                }
            }
            else
            {
                LED1(Bit_RESET);
                j=0;
            }
        }
        if(c_gc.MonitorDeviceNum>0 && GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==RESET &&NotConfiging==1) //没有仪器不采集,停电不采集
        {
            Server_Process(); 
        }
        if(c_gc.MonitorDeviceNum==0 && NotConfiging==1) //没有配置仪器时可以透传
        {
            Server_Process_Li();
        }
        if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==SET) //断电中
        {
            LED1(Bit_RESET);
            if(timer_check(PowerDownTimer)) //断电时间超过10秒
            {
                SMSAlarm(eAlarmType_PowerOff,0,0);
                PowerDownTimer=0;
            }
            if(c_gc.MonitorDeviceNum==0)
            {
                GPIO_ResetBits(GPIOC,GPIO_Pin_6); //掉电自杀
            }
            if(abuf->pNext==NULL && abuf->usable==0 && PowerDownTimer==0)
            {
                GPIO_ResetBits(GPIOC,GPIO_Pin_6); //掉电自杀
            }
        }
        else
        {
            timer_init(&PowerDownTimer,10000); //断电定时
        }
        SMSAlarm_Process();     //短信报警
        SMSAlarm_GSMProcess();
        SMSAlarm_GSMWorkStat();
        Client_Receive();
        if(timer_check(Led1Timer))
        {
            timer_init(&Led1Timer,1000);
            (l==0)?(l=1):(l=0);
            LED1(l);
        }
        //f_printf(logfile,"%d\r\n",g_tick_1ms);
        //f_sync(logfile);
	} 
    free(fs);
}
















