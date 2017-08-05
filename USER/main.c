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
//add ������־ ���� �ϴ�
// ���� �ϴ� ����ʽ��

volatile uint32_t g_tick_1ms;

uint32_t Led1Timer=0,Led1Timer2=0,Led2Timer=0,Led2Timer2=0,Led3Timer=0;
//uint32_t SMSAlarmTimer=1;
volatile _HostStat hstat;
volatile _GlobalConfig _gc;
const _GlobalConfig c_gc __attribute__((at(0x08010000)))={
    .AlarmIntervalTime=1,
    .OverLimitInterval=1,
    .OfflineAlarmInterval=1,
    .OverLimitONOFF=1,
    .AlarmONOFF=1,
    .OfflineAlarmONOFF=1,
    .SMSAlarmNum=0,
    .SamplingInterval=60,
};

FATFS *fs;
const _HardwareInfo _hi __attribute__((at(0x08001000)))={0x80000000,0xC000000,0xFEDCBA98};
const _DeviceConfig cDc[255] __attribute__((at(0x08011000)))={0};
const char MHID[]={"MH6001A001"};//__attribute__((at(0x08008000)))={"MH6001A001"};
_DeviceData _Dd[255]={0};
extern uint8_t *SMSAlarmMessage;
extern __mbuf *u1mbuf,*u2mbuf,*u3mbuf,*gmbuf;
extern __abuf *abuf;

extern uint16_t GSMWorkStat;

extern struct rtc_time systmtime;

extern uint8_t stat;

static void gpio_init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	 //ʹ��PB�˿�ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
    GPIO_Init(GPIOD, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOD.8
    GPIO_ResetBits(GPIOD,GPIO_Pin_8);						//485CTR
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOA, &GPIO_InitStructure);	
    GPIO_ResetBits(GPIOA,GPIO_Pin_4);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOC, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOD.8
    GPIO_ResetBits(GPIOC,GPIO_Pin_6);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;  //�ϵ��ж�
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);	
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;  //Line��
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; //RJ45����δ�ý�
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource7);
    EXTI_InitStructure.EXTI_Line=EXTI_Line7;
    EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd=ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn; //
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
uint32_t PowerDownTimer=0;
int main(void)
{	 
    uint8_t l=0,k=0,j=0;
    unsigned long long fre_clust,freespace;//,total;
    FRESULT fres=FR_INVALID_DRIVE;
    
    abuf=CreateAlarmbuf(60);
    memcpy((uint8_t*)&_gc,(uint8_t*)&c_gc,sizeof(_GlobalConfig));
    //_gc.RetryInterval=11;
    
    
    fs=malloc(1020);	    
    SysTick_Init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	gpio_init();
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
    RTC_Init();
	FLASH_Init();				//��ʼ��W25Q128 
    USART1_Init(115200);	 	//���ڳ�ʼ��Ϊ115200
    USART2_Init(115200);	 	//���ڳ�ʼ��Ϊ115200
    USART3_Init(19200);
	SD_Init();
    
    if(SD_CardIsInserted())
    {
        
        fres=f_mount(fs,"0:",0); 					//����SD�� 		
        if(fres!=FR_OK)
            goto startupsderro;
        fres=f_getfree("0:",&fre_clust,&fs); 					//���ʣ��ռ�
        if(fres==FR_OK)
        {
            //total = (fs->n_fatent - 2) * fs->csize;
            freespace = fre_clust * fs->csize;
            if(freespace<8192)      
                hstat.SDCardStat=20;         //SD���ռ䲻��
            else 
                hstat.SDCardStat=fres;
        }
        else
        {
            hstat.SDCardStat=fres;
        }
//        f_mount(0,"0:",1);
	}
    else    
    {
        startupsderro:
        hstat.SDCardStat=fres;
    }
    //timer_init(&Led3Timer,500);
    LED2(Bit_SET);
    //Usart2_SendData("AT+CSQ\r\n",8);
	while(1)
	{
        if(GSMWorkStat==eGSMStat_Ready)
        {
            if(timer_check_nolimit(Led2Timer))  //�豸������
            {
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
                timer_init(&Led2Timer,1000);
                (k==0)?(k=1):(k=0);
                LED2(k);
            }
        }
        if(hstat.SDCardStat!=FR_OK && Led1Timer==0)  //SD���쳣����
        {
            timer_init(&Led1Timer,400);
        }
        if(hstat.SDCardStat==FR_OK)
        {
            Led1Timer=0;
            
            if(stat==e_Stat_SampleingWait || stat==e_Stat_Sampling)
            {
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
            }
                
        }
        if(c_gc.MonitorDeviceNum>0 && GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==RESET) //û���������ɼ�,ͣ�粻�ɼ�
        {
            Server_Process(); 
        }
        if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)==SET) //�ϵ���
        {
            if(timer_check(PowerDownTimer)) //�ϵ�ʱ�䳬��10��
            {
                SMSAlarm(eAlarmType_PowerOff,0,0);
                PowerDownTimer=0;
            }
            if(c_gc.MonitorDeviceNum==0)
            {
                GPIO_ResetBits(GPIOC,GPIO_Pin_6); //������ɱ
            }
            if(abuf->pNext==NULL && abuf->usable==0 && PowerDownTimer==0)
            {
                GPIO_ResetBits(GPIOC,GPIO_Pin_6); //������ɱ
            }
        }
        else
        {
            timer_init(&PowerDownTimer,10000); //�ϵ綨ʱ
        }
        SMSAlarm_Process();     //���ű���
        SMSAlarm_GSMProcess();
        SMSAlarm_GSMWorkStat();
        Client_Receive();
        if(timer_check(Led1Timer))
        {
            timer_init(&Led1Timer,1000);
            (l==0)?(l=1):(l=0);
            LED1(l);
        }
        
	} 
    free(fs);
}
















