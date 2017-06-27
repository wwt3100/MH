
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

const uint32_t WLP_HEAD = 0x287b1601;
const uint32_t WLP_TAIL = 0x047e7d29;

uint8_t Server_Receive(void);
void Server_Send67(uint8_t *pID);
extern _DeviceConfig cDc[255];

extern __mbuf *u1mbuf,*u2mbuf,*u3mbuf,*gmbuf;
extern struct rtc_time systmtime;

extern volatile _GlobalConfig _gc;

static _DeviceData ts;
void Server_Process()
{
    static uint32_t timeout=0;
    static uint32_t SamplingIntervalTimer=1;
    static uint8_t stat=0,resend=0,i=0;
    if(timer_check(SamplingIntervalTimer))
    {
        switch(stat)
        {
            case 0:
                stat=1;
                //Server_Send67((cDc[i].ID));
                Server_Send67("HS300BS58F"); //for test
                timer_init(&timeout,_gc.RetryInterval*100);
                if(resend++>=2)
                {
                    i++;
                    resend=0;
                }
                break;
            case 1:
                if(Server_Receive()==1)
                {
                    if(strcmp((cDc[i].ID),"HS300BS58F"))  //for test
                    //if(memcmp(ts.ID,(cDc[i].ID),10)==0)
                    {
                        //do some
                        i++;
                        resend=0;
                        stat=0;
                    }
                }
                if(timer_check(timeout))
                {
                    stat=0;
                }
                break;
            default:
                break;
        }
        if(_gc.MonitorDeviceNum<=i)
        {
            i=0;
            timer_init(&SamplingIntervalTimer,_gc.SamplingInterval*1000); //设置采样间隔
        }
    }
}
void Server_Send67(uint8_t *pID)
{
    uint8_t *Sendbuff,Verify=0,i=0;
    
    Sendbuff=malloc(250);
    memset(Sendbuff,0,250);
    memcpy(Sendbuff,&WLP_HEAD,4);
    memcpy(Sendbuff+4,pID,10);
    Sendbuff[14]=0x67;
    for(i=0;i<11;i++)
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
    for(i=0;i<u3mbuf->datasize-4-4-1;i++) //计算校验
    {
        Verify = Verify ^ *(u3mbuf->pData+2+i);
    }
    if(Verify != *(u3mbuf->pData+2+i)) //校验错误
    {
        goto processend;
    }
//    if(memcmp(PRODUCT_SERISE,(u3mbuf->pData+2),10)==0)//如果是发给网关的信息
//    {
//        switch(*(u3mbuf->pData+12))
//        {
//            case 0x70:
//                IDLE_TIME=*(u3mbuf->pData+13)<<8;  //大端小端?
//                IDLE_TIME|=*(u3mbuf->pData+14);
//                IDLE_TIME*=1000;
//                WAIT_SYNCH_TIME=*(u3mbuf->pData+15)<<8;  //大端小端?
//                WAIT_SYNCH_TIME|=*(u3mbuf->pData+16);
//                WAIT_SYNCH_TIME*=1000;
//                WAIT_TRANS_TIME=*(u3mbuf->pData+17)<<8;  //大端小端?
//                WAIT_TRANS_TIME|=*(u3mbuf->pData+18);
//                WAIT_TRANS_TIME*=1000;
//                break;
//            default:
//                break;
//        }
//    }
//    else //其他信息
    {
        switch(*(u3mbuf->pData+12))
        {
            case 0x68:
                switch(u3mbuf->datasize) //按照字节数来区分协议
                {
                    case 25+4:  
                        memcpy(&(ts.ID[0]),u3mbuf->pData+2,10);
                        memcpy(&(ts.Data1),u3mbuf->pData+14,4);
                        memcpy(&(ts.Data2),u3mbuf->pData+18,2);
                        memcpy(&(ts.Data3),u3mbuf->pData+22,2);
                        ts.Data4=0;
                        ret=1;
                        break;
                    default:
                        break;
                }
                to_tm(RTC_GetCounter(),&systmtime);
                ts.time[0]=systmtime.tm_year-2000;
                ts.time[1]=systmtime.tm_mon ;
                ts.time[2]=systmtime.tm_mday;
                ts.time[3]=systmtime.tm_hour;
                ts.time[4]=systmtime.tm_min ;
                ts.time[5]=systmtime.tm_sec ;
                break;
            default:
                break;
        }
    }
    processend:
    tb=u3mbuf->pNext;
    u3mbuf->usable=0;
    free(u3mbuf);
    u3mbuf=tb;
    return ret;
}












