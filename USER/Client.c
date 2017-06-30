
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
static void Client_Rx30Tx31()
{
    uint8_t sendbuf[20]={0};
    
}
uint8_t Client_Receive()
{
    uint8_t i=0,Verify=0x00;
    __mbuf* tb;
    uint8_t ret=0;
    if(u1mbuf->usable!=1)
        return ret;
    for(i=0;i<u1mbuf->datasize-4-1;i++) //����У��
    {
        Verify = Verify ^ *(u1mbuf->pData+i);
    }
    if(Verify != *(u1mbuf->pData+i)) //У�����
    {
        goto clientprocessend;
    }
    if(memcmp(MHID,(u1mbuf->pData),10)==0)//���к���ͬ    //����Ƿ���������������Ϣ
    {
        switch(*(u1mbuf->pData+10))
        {
            case 0x30: 
                Client_Rx30Tx31(); //��������
                break;
            default:
                break;
        }
    }
    else if(u1mbuf->pData[0]==0)  //���к�Ϊ0 ����д���к�
    {
        switch(*(u1mbuf->pData+10))
        {
            case 0xCC:

                break;
            default:
                break;
        }
    }
    else //���������豸����Ϣ
    {
        hstat.ClientStat=CST_ClientHasData;
        return ret;
    }
    clientprocessend:
    tb=u1mbuf->pNext;
    u1mbuf->usable=0;
    free(u1mbuf);
    u1mbuf=tb;
    return ret;
}
