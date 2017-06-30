
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
extern uint8_t *SMSAlarmMessage;

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
                stat=e_Stat_SampleingWait;
                //Server_Send67((cDc[i].ID));
                Server_Send67("HS500BS657"); //for test
                timer_init(&timeout,_gc.RetryInterval*100);
                if(resend++>=2)  //�������3��û���յ��ظ�
                {
                    //���߱���
                    dev++;
                    resend=0;
                    if(_gc.MonitorDeviceNum<=dev)
                    {
                        dev=0;
                        timer_init(&SamplingIntervalTimer,_gc.SamplingInterval*1000); //���ò������
                        stat=e_Stat_Idle;
                    }
                }
            }
            break;
        case e_Stat_SampleingWait:
            if(Server_Receive()==1)
            {
                if(strcmp(((cDc[dev]).ID),"HS500BS657"))  //for test
                //if(memcmp(_Dd[dev].ID,(cDc[dev].ID),10)==0) //���ղɼ����ݵ���Ϣ
                {
                    //SaveData2RecodeFile(&cDc[device]);
                    SaveData2RecodeFile(&_Dd[dev]); 
                    SaveData2TempFile(&_Dd[dev]);
                    if(_Dd[dev].Data1>cDc[dev].Data1Max || _Dd[dev].Data1<cDc[dev].Data1Min)
                    {
                        timer_init(&_Dd[dev].Data1AlarmTimer,_gc.AlarmIntervalTime*1000);
                    }
                    else
                    {
                        _Dd[dev].Data1AlarmTimer=0;
                        if(_Dd[dev].Alram[0]==1)
                        {
                            //���ű������
                        }
                    }
                    if(_Dd[dev].Data2>cDc[dev].Data2Max || _Dd[dev].Data2<cDc[dev].Data2Min)
                    {
                        timer_init(&_Dd[dev].Data2AlarmTimer,_gc.AlarmIntervalTime*1000);
                    }
                    else
                    {
                        _Dd[dev].Data2AlarmTimer=0;
                        if(_Dd[dev].Alram[1]==1)
                        {
                            //���ű������
                        }
                    }
                    if(_Dd[dev].Data3>cDc[dev].Data3Max || _Dd[dev].Data3<cDc[dev].Data3Min)
                    {
                        timer_init(&_Dd[dev].Data3AlarmTimer,_gc.AlarmIntervalTime*1000);
                    }
                    else
                    {
                        _Dd[dev].Data3AlarmTimer=0;
                        if(_Dd[dev].Alram[2]==1)
                        {
                            //���ű������
                        }
                    }
                    if(_Dd[dev].Data4>cDc[dev].Data4Max || _Dd[dev].Data4<cDc[dev].Data4Min)
                    {
                        timer_init(&_Dd[dev].Data4AlarmTimer,_gc.AlarmIntervalTime*1000);
                    }
                    else
                    {
                        _Dd[dev].Data4AlarmTimer=0;
                        if(_Dd[dev].Alram[3]==1)
                        {
                            //���ű������
                        }
                    }
                    if(timer_check(_Dd[dev].Data1AlarmTimer) && _Dd[dev].Alram[0]==0)
                    {
                        _Dd[dev].Alram[0]=1;
                        //���ű���
                    }
                    if(timer_check(_Dd[dev].Data2AlarmTimer) && _Dd[dev].Alram[1]==0)
                    {
                        _Dd[dev].Alram[1]=1;
                        //���ű���
                    }
                    if(timer_check(_Dd[dev].Data3AlarmTimer) && _Dd[dev].Alram[2]==0)
                    {
                        _Dd[dev].Alram[2]=1;
                        //���ű���
                    }
                    if(timer_check(_Dd[dev].Data4AlarmTimer) && _Dd[dev].Alram[3]==0)
                    {
                        _Dd[dev].Alram[3]=1;
                        //���ű���
                    }
                    dev++;
                    resend=0;
                    stat=e_Stat_Sampling;
                }
                else //������յ�����������Ϣ
                {
                    hstat.ServerStat=SST_ServerRun;
                    stat=e_Stat_Sampling;
                }
            }
            if(_gc.MonitorDeviceNum<=dev)
            {
                dev=0;
                timer_init(&SamplingIntervalTimer,_gc.SamplingInterval*1000); //���ò������
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
                if(timer_check(SamplingIntervalTimer))
                {
                    dev=0;
                    stat=e_Stat_Sampling;
                    timer_init(&SamplingIntervalTimer,_gc.SamplingInterval*1000); //���ò������
                    if(SMSAlarmMessage!=NULL)
                    {
                        SMSAlarmMessage=malloc(248);
                    }
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
                timer_init(&PCmsgtimeout,_gc.RetryInterval*100);
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
void Server_Send67(uint8_t *pID)
{
    uint8_t *Sendbuff,Verify=0,i=0;
    
    Sendbuff=malloc(248);
    memset(Sendbuff,0,248);
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
    uint8_t i=0,Verify=0x00;
    __mbuf* tb;
    uint8_t ret=0;
    if(u3mbuf->usable!=1)
        return ret;
    for(i=0;i<u3mbuf->datasize-4-1;i++) //����У��
    {
        Verify = Verify ^ *(u3mbuf->pData+i);
    }
    if(Verify != *(u3mbuf->pData+i)) //У�����
    {
        goto processend;
    }
    {
        switch(*(u3mbuf->pData+10))
        {
            case 0x68:
                switch(u3mbuf->datasize) //�����ֽ���������Э��
                {
                    case 25:  
                        memcpy(&(_Dd[dev].ID[0]),u3mbuf->pData,10);
                        memcpy(&(_Dd[dev].Data1),u3mbuf->pData+12,4);
                        memcpy(&(_Dd[dev].Data2),u3mbuf->pData+16,2);
                        _Dd[dev].Data3=0;
                        _Dd[dev].Data4=0;
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
    Usart1_SendData((uint8_t*)&WLP_HEAD,4); //Send to PC
    Usart1_SendData(u3mbuf->pData,u3mbuf->datasize);//Send to PC
    tb=u3mbuf->pNext;
    u3mbuf->usable=0;
    free(u3mbuf);
    u3mbuf=tb;
    return ret;
}












