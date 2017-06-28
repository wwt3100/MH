
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
#include "Client.h"

extern const uint32_t WLP_HEAD;
extern const uint32_t WLP_TAIL;

extern _DeviceConfig cDc[255];
extern _DeviceData _Dd[255];

extern __mbuf *u1mbuf,*u2mbuf,*u3mbuf,*gmbuf;
extern struct rtc_time systmtime;

extern volatile _GlobalConfig _gc;
extern _HostStat hstat;

extern const char MHID[12];
uint8_t Client_Receive()
{
    uint8_t i=0,Verify=0x00;
    __mbuf* tb;
    uint8_t ret=0;
    if(u1mbuf->usable!=1)
        return ret;
    for(i=0;i<u1mbuf->datasize-4-4-1;i++) //计算校验
    {
        Verify = Verify ^ *(u1mbuf->pData+i);
    }
    if(Verify != *(u3mbuf->pData+i)) //校验错误
    {
        goto clientprocessend;
    }
    if(memcmp(MHID,(u3mbuf->pData),10)==0)//如果是发给管理主机的信息
    {
        switch(*(u3mbuf->pData+10))
        {
            case 0x70:

                break;
            default:
                break;
        }
    }
    else //发给其他设备的信息
    {
        if(hstat.ServerStat==SST_ServerIDLE)
        {
            Usart3_SendData((uint8_t*)&WLP_TAIL,4); //Send to PC
            Usart3_SendData(u1mbuf->pData,u1mbuf->datasize);//Send to PC
        }
        else
        {
            hstat.ClientStat=CST_ClientHasData;
            return ret;
        }
    }
    clientprocessend:
    hstat.ClientStat=CST_ClientNoData;
    tb=u1mbuf->pNext;
    u1mbuf->usable=0;
    free(u1mbuf);
    u1mbuf=tb;
    return ret;
}