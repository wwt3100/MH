
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
extern const _GlobalConfig c_gc;
extern _HostStat hstat;

extern const char MHID[12];

///////////////////////////////////////////////////////////////
// 设置命令
//
static void Client_Rx30Tx31()
{
    uint8_t i=0,Verify=0;
    uint8_t sendbuf[20]={0};
    memcpy((uint8_t*)_gc.PhoneNumber1,u1mbuf->pData+12,88);
    STMFLASH_Write((uint32_t)&c_gc,(uint16_t*)&_gc,sizeof(_GlobalConfig));
    memcpy(sendbuf,&WLP_HEAD,4);
    memcpy(sendbuf+4,MHID,10);
    sendbuf[14]=0x31;
    sendbuf[15]=0x01;
    for(i=4;i<15;i++)   //i=0  =>  i=4
    {
        Verify = Verify ^ (sendbuf[i]);
    }
    sendbuf[16]=Verify;
    memcpy(sendbuf+16,&WLP_TAIL,4);
    Usart1_SendData(sendbuf,21);
}

///////////////////////////////////////////////////////////////
// 读取设置信息
//
static void Client_Rx32Tx33()
{
    uint8_t i=0,Verify=0;
    uint8_t *sendbuf;
    uint32_t num;
    FSIZE_t size;
    sendbuf=malloc(248);
    memset(sendbuf,0,248);
    memcpy(sendbuf,&WLP_HEAD,4);
    memcpy(sendbuf+4,MHID,10);
    sendbuf[14]=0x33;
    sendbuf[15]=0x01;
    memcpy(sendbuf+15,(uint8_t*)_gc.PhoneNumber1,89);
    ReadTempFileSize(&size);
    num=size/18;
    memcpy(sendbuf+15+89,&num,4);
    for(i=4;i<15+89;i++)   //i=0  =>  i=4
    {
        Verify = Verify ^ (sendbuf[i]);
    }
    sendbuf[16+89]=Verify;
    memcpy(sendbuf+16+89,&WLP_TAIL,4);
    Usart1_SendData(sendbuf,21);
}
///////////////////////////////////////////////////////////////
// 配置仪器
//
static void Client_Rx34Tx35()
{
    uint8_t i=0,Verify=0;
    uint8_t sendbuf[20]={0};
    
}

///////////////////////////////////////////////////////////////
// 时间同步
//
static void Client_Rx76Tx77()
{
    uint8_t i=0,Verify=0;
    uint8_t sendbuf[20]={0};
    
    //to do
    
    memcpy(sendbuf,&WLP_HEAD,4);
    memcpy(sendbuf+4,MHID,10);
    sendbuf[14]=0x77;
    sendbuf[15]=0x01;
    for(i=4;i<15;i++)   //i=0  =>  i=4
    {
        Verify = Verify ^ (sendbuf[i]);
    }
    sendbuf[16]=Verify;
    memcpy(sendbuf+16,&WLP_TAIL,4);
    Usart1_SendData(sendbuf,21);
}

///////////////////////////////////////////////////////////////
// 数据下载
//
static void Client_Rx5BTx5C()
{
    uint8_t i=0,Verify=0;
    uint8_t *sendbuf;
    uint32_t num,savenum;
    FSIZE_t size;
    sendbuf=malloc(248);
    memcpy(sendbuf,u1mbuf->pData+11,4);
    ReadTempFileSize(&size);
    savenum=size/18;
    
    
}

///////////////////////////////////////////////////////////////
// 报警记录下载
//
static void Client_Rx5DTx5E()
{
    uint8_t i=0,Verify=0;
    uint8_t *sendbuf;
    uint32_t num,savenum;
    FSIZE_t size;
    sendbuf=malloc(248);
    memcpy(sendbuf,u1mbuf->pData+11,4);
    ReadTempFileSize(&size);
    savenum=size/18;
    
    
}

///////////////////////////////////////////////////////////////
// 数据删除
//
static void Client_Rx72Tx73()
{
    FATFS *fs;
    uint8_t i=0,Verify=0;
    uint8_t sendbuf[20]={0};
    uint8_t fres;
    uint16_t cmd;
    char filename[25]={0};
    memcpy(&cmd,u1mbuf->pData+11,2);
    sendbuf[15]=0x00;
    if(SD_CardIsInserted())
    {
        fs = malloc(sizeof (FATFS));
        fres=f_mount(fs, "0:", 0);
        if(fres==FR_OK)
        {
            if(cmd==0x1991 ||cmd==0x9999)
            {
                fres=f_unlink(".Tempdata");
            }
            if(cmd==0x2992 || cmd==0x9999)
            {
                fres=f_unlink(".Alarmdata");
            }
            if(cmd==0x3993 || cmd==0x9999)
            {
                for(i=0;i<_gc.MonitorDeviceNum;i++)
                {
                    memcpy(filename,_Dd[i].ID,10);
                    strcat(filename,".xls");
                    fres=f_unlink(filename);
                }
            }
            if(fres==FR_NO_FILE || fres==FR_OK)
            {
                sendbuf[15]=1;
            }
        }
        free(fs);
    }
    memcpy(sendbuf,&WLP_HEAD,4);
    memcpy(sendbuf+4,MHID,10);
    sendbuf[14]=0x73;
    //sendbuf[15]=0x01;
    for(i=4;i<15;i++)   //i=0  =>  i=4
    {
        Verify = Verify ^ (sendbuf[i]);
    }
    sendbuf[16]=Verify;
    memcpy(sendbuf+16,&WLP_TAIL,4);
    Usart1_SendData(sendbuf,21);
}

uint8_t Client_Receive()
{
    uint8_t i=0,Verify=0x00;
    __mbuf* tb;
    uint8_t ret=0;
    if(u1mbuf->usable!=1)
        return ret;
    for(i=0;i<u1mbuf->datasize-4-1;i++) //计算校验
    {
        Verify = Verify ^ *(u1mbuf->pData+i);
    }
    if(Verify != *(u1mbuf->pData+i)) //校验错误
    {
        goto clientprocessend;
    }
    if(memcmp(MHID,(u1mbuf->pData),10)==0)//序列号相同    //如果是发给管理主机的信息
    {
        switch(*(u1mbuf->pData+10))
        {
            case 0x30: 
                Client_Rx30Tx31(); //设置命令
                break;
            case 0x32:
                Client_Rx32Tx33(); //读取设置信息
                break;
            default:
                break;
        }
    }
    else if(u1mbuf->pData[0]==0)  //序列号为0 重新写序列号
    {
        switch(*(u1mbuf->pData+10))
        {
            case 0xCC:

                break;
            default:
                break;
        }
    }
    else //发给其他设备的信息
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
