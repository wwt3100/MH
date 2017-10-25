
#include "485Server.h"
#include "stm32f10x.h"                  // Device header
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
#include "DataFile.h"


const uint32_t WLP_HEAD = 0x287b1601;
const uint32_t WLP_TAIL = 0x047e7d29;

uint8_t Server_Receive(void);
void Server_Send67(uint8_t *pID);
extern _DeviceConfig cDc[255];
extern _DeviceData _Dd[255];

extern __mbuf *u1mbuf,*u2mbuf,*u3mbuf,*gmbuf;
extern struct rtc_time systmtime;

extern volatile _GlobalConfig _gc;
extern volatile _HostStat hstat;
_DeviceData ts;

volatile uint8_t stat=0,resend=0,dev=0;
uint32_t timeout=0,PCmsgtimeout=0;
uint32_t SamplingIntervalTimer=1;
uint32_t OneSecTimer=0;
extern uint8_t *SMSAlarmMessage;
uint8_t MustSave=0;
uint8_t Server_Process()
{
    __mbuf* tb;
    static uint8_t laststat=0;
    switch(stat)
    {
        case e_Stat_Sampling:
            if(hstat.ClientStat==CST_ClientHasData)
            {
                laststat=e_Stat_Sampling;
                stat=e_Stat_PCMessage;
            }
            else
            {
                if(resend>=3)  //如果发送2次没有收到回复
                {
                    
                    if(_Dd[dev].Alram[0]==0 && _Dd[dev].OfflineAlarmTimer==0)
                    {
                        timer_init_sp(&(_Dd[dev].OfflineAlarmTimer),_gc.OfflineAlarmInterval*60000);
                    }
                    dev++;
                    resend=0;
                    if(_gc.MonitorDeviceNum<=dev)
                    {
                        dev=0;
                        //timer_init(&SamplingIntervalTimer,_gc.SamplingInterval*1000); //设置采样间隔
                        stat=e_Stat_Idle;
                    }
                }
                else
                {
                    Server_Send67((cDc[dev].ID));
                    timer_init(&timeout,15*100);
                    stat=e_Stat_SampleingWait;
                    resend++;
                }
            }
            break;
        case e_Stat_SampleingWait:
            if(Server_Receive()==1)
            {
                //if(strcmp(((cDc[dev]).ID),"HS500BS657"))  //for test
                if(memcmp(_Dd[dev].ID,(cDc[dev].ID),10)==0) //接收采集数据的信息
                {

//                    SaveData2RecodeFile(&_Dd[dev]); 
                    SaveData2TempFile(&_Dd[dev]);
                    
                    _Dd[dev].OfflineAlarmTimer=0; 
                    
                    if(_Dd[dev].Data1>cDc[dev].Data1Max || _Dd[dev].Data1<cDc[dev].Data1Min)
                    {
                        if(_Dd[dev].Data1AlarmTimer==0)
                            timer_init_sp(&_Dd[dev].Data1AlarmTimer,_gc.OverLimitInterval*60000+2000);
                    }
                    else
                    {
                        _Dd[dev].Data1AlarmTimer=0;
                    }
                    if(_Dd[dev].Data2>cDc[dev].Data2Max || _Dd[dev].Data2<cDc[dev].Data2Min)
                    {
                        if(_Dd[dev].Data2AlarmTimer==0)
                            timer_init_sp(&_Dd[dev].Data2AlarmTimer,_gc.OverLimitInterval*60000+2000);
                    }
                    else
                    {
                        _Dd[dev].Data2AlarmTimer=0;
                    }
                    if((_Dd[dev].Data1AlarmTimer+_Dd[dev].Data2AlarmTimer!=0)||(systmtime.tm_min%5==0))  //超限 保存时间和采集时间一样
                    {
                        SaveData2RecodeFile(&_Dd[dev]); 
                        //SaveData2TempFile(&_Dd[dev]);
                    }
                    dev++;
                    resend=0;
                    stat=e_Stat_Sampling;
                }
                else //如果接收到的是其他信息
                {
                    hstat.ServerStat=SST_ServerRun;
                    stat=e_Stat_Sampling;
                }
            }
            if(_gc.MonitorDeviceNum<=dev)
            {
                dev=0;
                //timer_init(&SamplingIntervalTimer,_gc.SamplingInterval*1000); //设置采样间隔
                stat=e_Stat_Idle;
            }
            else
            {
                if(timer_check(timeout))
                {
                    stat=e_Stat_Sampling;
                }
            }
            break;
        case e_Stat_Idle:
            if(hstat.ClientStat==CST_ClientHasData)
            {
                laststat=e_Stat_Idle;
                stat=e_Stat_PCMessage;
            }
            else
            {
                if(timer_check(SamplingIntervalTimer)&&(RTC_GetCounter()%60==0))
                {
                    dev=0;
                    stat=e_Stat_Sampling;
                    timer_init(&SamplingIntervalTimer,_gc.SamplingInterval*60000-5000); //设置采样间隔
                    //500ms是否会导致偏差无法修正?
                }
            }
            break;
        case e_Stat_PCMessage:
            if(u1mbuf->usable==1)
            {
                __485SetSend();
                Usart3_SendData((uint8_t*)&WLP_HEAD,4); //Send PC Message
                Usart3_SendData(u1mbuf->pData,u1mbuf->datasize);//Send PC Message
                __485SetReceive();
                tb=u1mbuf->pNext;
                u1mbuf->usable=0;
                free(u1mbuf);
                u1mbuf=tb;
                stat=e_Stat_PCMessageWait;
                timer_init(&PCmsgtimeout,20*100);
            }
            else
            {
                hstat.ClientStat=CST_ClientNoData;
                stat=laststat;
            }
            break;
        case e_Stat_PCMessageWait:
            if(Server_Receive()==1 || timer_check(PCmsgtimeout))
            {
                stat=laststat;
                hstat.ClientStat=CST_ClientNoData;
            }
            break;
        default:
            stat=1;
            break;
    }
    
    return stat;
}
uint8_t Server_Process_Li()
{
    __mbuf* tb;
    static uint8_t laststat=0;
    static uint8_t stat_l=e_Stat_Idle;
    switch(stat_l)
    {
        case e_Stat_Idle:
            if(hstat.ClientStat==CST_ClientHasData)
            {
                laststat=e_Stat_Idle;
                stat_l=e_Stat_PCMessage;
            }
            break;
        case e_Stat_PCMessage:
            if(u1mbuf->usable==1)
            {
                __485SetSend();
                Usart3_SendData((uint8_t*)&WLP_HEAD,4); //Send PC Message
                Usart3_SendData(u1mbuf->pData,u1mbuf->datasize);//Send PC Message
                __485SetReceive();
                tb=u1mbuf->pNext;
                u1mbuf->usable=0;
                free(u1mbuf);
                u1mbuf=tb;
                stat_l=e_Stat_PCMessageWait;
                timer_init(&PCmsgtimeout,20*100);
            }
            else
            {
                hstat.ClientStat=CST_ClientNoData;
                stat=laststat;
            }
            break;
        case e_Stat_PCMessageWait:
            if(Server_Receive()==1 || timer_check(PCmsgtimeout))
            {
                stat_l=laststat;
                hstat.ClientStat=CST_ClientNoData;
            }
            break;
        default:
            stat_l=e_Stat_Idle;
            break;
    }
    return stat;
}
void Server_Send67(uint8_t *pID)
{
    uint8_t *Sendbuff,Verify=0,i=0;
    
    Sendbuff=malloc(252);
    memset(Sendbuff,0,252);
    memcpy(Sendbuff,&WLP_HEAD,4);
    memcpy(Sendbuff+4,pID,10);
    Sendbuff[14]=0x67;
    for(i=4;i<11;i++)   //i=0  =>  i=4
    {
        Verify = Verify ^ (Sendbuff[i]);
    }
    Sendbuff[15]=Verify;
    memcpy(Sendbuff+16,&WLP_TAIL,4);
    __485SetSend();
    Usart3_SendData(Sendbuff,20);
    __485SetReceive();
    free(Sendbuff);
}
uint8_t Server_Receive()
{
    uint16_t i=0;
    uint8_t Verify=0x00;
    __mbuf* tb;
    uint8_t ret=0;
    if(u3mbuf->usable!=1)
        return ret;
    for(i=0;i<u3mbuf->datasize-4-1;i++) //计算校验
    {
        Verify = Verify ^ *(u3mbuf->pData+i);
    }
    if(Verify != *(u3mbuf->pData+i)) //校验错误
    {
        goto processend;
    }
    {
        switch(*(u3mbuf->pData+10))
        {
            case 0x68:
                switch(u3mbuf->datasize) //按照字节数来区分协议
                {
                    case 25:  
                        memcpy(&(_Dd[dev].ID[0]),u3mbuf->pData,10);
                        _Dd[dev].ID[11]=*(u3mbuf->pData+11);            //ID多余的byte存储仪器类型
                        memcpy(&(_Dd[dev].Data1),u3mbuf->pData+12,4);
                        memcpy(&(_Dd[dev].Data2),u3mbuf->pData+16,2);
                        ret=1;
                        break;
                    default:
                        break;
                }
                to_tm(RTC_GetCounter(),&systmtime);
                _Dd[dev].time[0]=systmtime.tm_year-2000;
                _Dd[dev].time[1]=systmtime.tm_mon ;
                _Dd[dev].time[2]=systmtime.tm_mday;
                _Dd[dev].time[3]=systmtime.tm_hour;
                _Dd[dev].time[4]=systmtime.tm_min ;
                _Dd[dev].time[5]=systmtime.tm_sec ;
                break;
            default:
                break;
        }
    }
    processend:
    //if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_7)==SET)  //Link脚不管用???
    {
        Usart1_SendData((uint8_t*)&WLP_HEAD,4); //Send to PC
        Usart1_SendData(u3mbuf->pData,u3mbuf->datasize);//Send to PC
    }
    tb=u3mbuf->pNext;
    u3mbuf->usable=0;
    free(u3mbuf);
    u3mbuf=tb;
    return ret;
}












