
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
extern _GlobalConfig c_gc;
extern _HostStat hstat;

extern const char MHID[12];

extern uint32_t SamplingIntervalTimer;

extern FATFS *fs;
///////////////////////////////////////////////////////////////
// 设置命令 Check OK
//
// Add PhoneNum Check
// Add SMSAlarmNum Limit
//
static void Client_Rx30Tx31()
{
    uint8_t i=0,Verify=0;
    uint8_t sendbuf[25]={0};
    memcpy((uint8_t*)_gc.PhoneNumber,u1mbuf->pData+12,88);
    sendbuf[15]=0x01;
    for(i=0;i<5;i++)
    {
        if((_gc.PhoneNumber[i][0]<='0'||_gc.PhoneNumber[i][0]>='9') && _gc.PhoneNumber[i][0]!='+' && _gc.PhoneNumber[i][0]!=0 )
        {
            sendbuf[15]=0x00;
        }
    }
    if(_gc.SMSAlarmNum>10) //短信重发次数限制10次
    {
        _gc.SMSAlarmNum=10;
    }
    STMFLASH_Write((uint32_t)&c_gc,(uint16_t*)&_gc,sizeof(_GlobalConfig));
    memcpy(sendbuf,&WLP_HEAD,4);
    memcpy(sendbuf+4,MHID,10);
    sendbuf[14]=0x31;
    //sendbuf[15]=0x01;
    
    for(i=4;i<15;i++)   //i=0  =>  i=4
    {
        Verify = Verify ^ (sendbuf[i]);
    }
    sendbuf[16]=Verify;
    memcpy(sendbuf+17,&WLP_TAIL,4);
    Usart1_SendData(sendbuf,21);
    SamplingIntervalTimer=1;//开始采集
}

///////////////////////////////////////////////////////////////
// 读取设置信息 Check OK
//
static void Client_Rx32Tx33()
{
    uint16_t i=0;
    uint8_t Verify=0;
    uint8_t *sendbuf;
    uint32_t num;
    FSIZE_t size;
    sendbuf=malloc(252);
    if(sendbuf==NULL)
        return ;
    memset(sendbuf,0,252);
    memcpy(sendbuf,&WLP_HEAD,4);
    memcpy(sendbuf+4,MHID,10);
    sendbuf[14]=0x33;
    sendbuf[15]=0x01;
    memcpy((uint8_t*)&_gc,(uint8_t*)&c_gc,sizeof(_GlobalConfig));
    memcpy(sendbuf+16,(uint8_t*)_gc.PhoneNumber,89);
    ReadTempFileSize(&size);
    num=size/18;
    memcpy(sendbuf+16+89,&num,4);
    for(i=4;i<16+89+4;i++)   //i=0  =>  i=4
    {
        Verify = Verify ^ *(sendbuf+i);
    }
    sendbuf[16+89+4]=Verify;
    memcpy(sendbuf+16+89+4+1,&WLP_TAIL,4);
    Usart1_SendData(sendbuf,16+89+1+4+4);
    free(sendbuf);
}
///////////////////////////////////////////////////////////////
// 配置仪器 Check OK
//
static void Client_Rx34Tx35()
{
    static uint8_t sen;
    int16_t t16;
    uint8_t i=0,Verify=0;
    uint8_t sendbuf[25]={0};
    uint8_t num;
    _DeviceConfig t_dc[4]={0};
    sen=u1mbuf->pData[13];
    num=u1mbuf->pData[14];
    if(u1mbuf->pData[12]==sen)
    {
        _gc.MonitorDeviceNum=((sen-1)*4)+num;
        STMFLASH_Write((uint32_t)&c_gc,(uint16_t*)&_gc,sizeof(_GlobalConfig));
        SamplingIntervalTimer=1;//开始采集  //fix 当全部配置完成之后再采集
    }
    for(i=0;i<num;i++)
    {
        memcpy(t_dc[i].DeviceName,u1mbuf->pData+(48*i)+15,24);
        memcpy(t_dc[i].ID,u1mbuf->pData+(48*i)+15+24,10);
        memcpy(&t16,u1mbuf->pData+(48*i)+39+10,2);
        t_dc[i].Data1Max=t16;
        memcpy(&t16,u1mbuf->pData+(48*i)+39+12,2);
        t_dc[i].Data1Min=t16;
        memcpy(&t16,u1mbuf->pData+(48*i)+39+14,2);
        t_dc[i].Data2Max=t16;
        memcpy(&t16,u1mbuf->pData+(48*i)+39+16,2);
        t_dc[i].Data2Min=t16;
    }
    STMFLASH_Write((uint32_t)&cDc[4*(sen-1)],(uint16_t*)t_dc,sizeof(_DeviceConfig)*num);
    
    
    memcpy(sendbuf,&WLP_HEAD,4);
    memcpy(sendbuf+4,MHID,10);
    sendbuf[14]=0x35;
    sendbuf[15]=0x01;
    for(i=4;i<15;i++)   //i=0  =>  i=4
    {
        Verify = Verify ^ (sendbuf[i]);
    }
    sendbuf[16]=Verify;
    memcpy(sendbuf+17,&WLP_TAIL,4);
    Usart1_SendData(sendbuf,21);
    
}

///////////////////////////////////////////////////////////////
// 时间同步   Check OK
//
// add fix BCD -> Hex
// fix bug systemtime day -2
// add rtc time check
//
static void Client_Rx76Tx77()
{
    uint8_t i=0,Verify=0;
    uint8_t sendbuf[25]={0};
    uint8_t *AppData=u1mbuf->pData+12;
    int16_t checktime=0;

    systmtime.tm_sec =RTC_CONVERT_BCD2BIN(*AppData);
    AppData++;
    systmtime.tm_min =RTC_CONVERT_BCD2BIN(*AppData);
    AppData++;
    systmtime.tm_hour=RTC_CONVERT_BCD2BIN(*AppData);
    AppData++;
    systmtime.tm_mday=RTC_CONVERT_BCD2BIN(*AppData);
    AppData++;
    systmtime.tm_mon =RTC_CONVERT_BCD2BIN(*AppData);
    AppData+=2;
    systmtime.tm_year=RTC_CONVERT_BCD2BIN(*AppData)+2000;
    
    checktime+=systmtime.tm_min;
    checktime+=systmtime.tm_hour;
    checktime+=systmtime.tm_mday;
    checktime+=systmtime.tm_mon;
    checktime+=systmtime.tm_year;
    
    RTC_Config();	  /* RTC Configuration */
    RTC_SetCounter(mktimev(systmtime));
    RTC_WaitForLastTask();
    BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    to_tm(RTC_GetCounter(), &systmtime);
    checktime-=systmtime.tm_min;
    checktime-=systmtime.tm_hour;
    checktime-=systmtime.tm_mday;
    checktime-=systmtime.tm_mon;
    checktime-=systmtime.tm_year;
    if(abs(checktime)<3) //简单校验
    {
        sendbuf[15]=0x01;
    }
    else
    {
        sendbuf[15]=0x00;
    }
    memcpy(sendbuf,&WLP_HEAD,4);
    memcpy(sendbuf+4,MHID,10);
    sendbuf[14]=0x77;
    for(i=4;i<15;i++)   //i=0  =>  i=4
    {
        Verify = Verify ^ (sendbuf[i]);
    }
    sendbuf[16]=Verify;
    memcpy(sendbuf+17,&WLP_TAIL,4);
    Usart1_SendData(sendbuf,21);
}

///////////////////////////////////////////////////////////////
// 数据下载   Check OK
//
static void Client_Rx78Tx79()
{
//    FATFS *fs;     /* Ponter to the filesystem object */
    FRESULT fres=FR_NOT_READY;
    FIL *fp;
    uint8_t i=0,Verify=0;
    uint8_t *sendbuf;
    uint32_t savenum=0;
    uint32_t allpack=0;
    static uint32_t loclpack;
    FSIZE_t size=0;
    sendbuf=malloc(252);
    memset(sendbuf,0,252);
    memcpy(&loclpack,u1mbuf->pData+11,4);
    memcpy(sendbuf,&WLP_HEAD,4);
    memcpy(sendbuf+4,MHID,10);
    sendbuf[14]=0x79;
    sendbuf[15]=0x00;
    if(SD_CardIsInserted())
    {
//        fs = malloc(636);//malloc(sizeof (FATFS));
        fp = malloc(636);
        fres=f_mount(fs, "0:", 0);
        if(fres==FR_OK)
        {
            fres=f_open(fp,".Tempdata",FA_OPEN_EXISTING | FA_WRITE | FA_READ);
            switch(fres)
            {
                case FR_OK:
                    size=f_size(fp);
                    savenum=size/18;
                    break;
                default:
                    break;
            }
            allpack=savenum/10;
            if(savenum%10!=0)
            {
                allpack++;
            }
            if(loclpack-1>allpack)  //判断是否跃包
            {
                sendbuf[15]=0x00;
            }
            f_lseek(fp,(loclpack-1)*180);      //fix bug 文件指针移动位置错误
            f_read(fp,sendbuf+25,180,&savenum);
            sendbuf[24]=savenum/18;  //包中数据数量 重复利用savenum内存
            f_close(fp);
//            f_mount(0,"0:",0);
        }
        if(savenum!=0)
        {
            sendbuf[15]=0x01;
        }
        free(fp);
//        free(fs);
    }
    memcpy(sendbuf+16,&allpack,4);
    memcpy(sendbuf+20,&loclpack,4);
    for(i=4;i<14+2+8+savenum+1;i++)   //i=0  =>  i=4
    {
        Verify = Verify ^ (sendbuf[i]);
    }
    sendbuf[14+2+8+savenum+1]=Verify;
    memcpy(sendbuf+14+2+8+savenum+2,&WLP_TAIL,4);
    Usart1_SendData(sendbuf,14+2+8+savenum+2+4);
    free(sendbuf);
}

///////////////////////////////////////////////////////////////
// 报警记录下载
//
static void Client_Rx80Tx81()
{
//    FATFS *fs;     /* Ponter to the filesystem object */
    FRESULT fres=FR_NOT_READY;
    FIL *fp;
    uint8_t i=0,Verify=0;
    uint8_t *sendbuf;
    uint32_t savenum=0;
    uint32_t allpack=0;
    static uint32_t loclpack;
    FSIZE_t size=0;
    sendbuf=malloc(252);
    memset(sendbuf,0,252);
    memcpy(&loclpack,u1mbuf->pData+11,4);
    memcpy(sendbuf,&WLP_HEAD,4);
    memcpy(sendbuf+4,MHID,10);
    sendbuf[14]=0x81;
    sendbuf[15]=0x00;
    if(SD_CardIsInserted())
    {
//        fs = malloc(636);//malloc(sizeof (FATFS));
        fp = malloc(636);
        fres=f_mount(fs, "0:", 0);
        if(fres==FR_OK)
        {
            fres=f_open(fp,".Alarmdata",FA_OPEN_EXISTING | FA_WRITE | FA_READ);
            switch(fres)
            {
                case FR_OK:
                    size=f_size(fp);
                    savenum=size/44;
                    break;
                default:
                    break;
            }
            allpack=savenum/5;
            if(savenum%10!=0)
            {
                allpack++;
            }
            if(loclpack-1>allpack)  //判断是否跃包
            {
                sendbuf[15]=0x00;
            }
            f_lseek(fp,(loclpack-1)*220);  //fix bug 文件指针移动位置错误
            f_read(fp,sendbuf+25,220,&savenum);
            sendbuf[24]=savenum/44;  //包中数据数量 重复利用savenum内存      
            f_close(fp);
//            f_mount(0,"0:",0);
        }
        if(savenum!=0)
        {
            sendbuf[15]=0x01;
        }
        
//        free(fs);
        free(fp);
    }
    memcpy(sendbuf+16,&allpack,4);
    memcpy(sendbuf+20,&loclpack,4);
    for(i=4;i<14+2+8+savenum+1;i++)   //i=0  =>  i=4
    {
        Verify = Verify ^ (sendbuf[i]);
    }
    sendbuf[14+2+8+savenum+1]=Verify;
    memcpy(sendbuf+14+2+8+savenum+2,&WLP_TAIL,4);
    Usart1_SendData(sendbuf,14+2+8+savenum+2+4);
    free(sendbuf);
}

///////////////////////////////////////////////////////////////
// 数据删除  Check OK
// new form
//
static void Client_Rx72Tx73()
{
//    FATFS *fs;
    uint8_t i=0,Verify=0;
    uint8_t sendbuf[20]={0};
    uint8_t fres;
    uint16_t cmd;
    char filename[25]={0};
    cmd=*u1mbuf->pData+11;
    cmd<<=8;
    cmd|=*u1mbuf->pData+12;
    memcpy(&cmd,u1mbuf->pData+11,2);
    sendbuf[15]=0x00;
    if(SD_CardIsInserted())
    {
//        fs = malloc(636);//malloc(sizeof (FATFS));
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
//        free(fs);
    }
    memcpy(sendbuf,&WLP_HEAD,4);
    memcpy(sendbuf+4,MHID,10);
    sendbuf[14]=0x73;
    sendbuf[15]=0x01;
    sendbuf[16]=cmd>>8;
    sendbuf[17]=cmd&0x00ff;
    for(i=4;i<17;i++)   //i=0  =>  i=4
    {
        Verify = Verify ^ (sendbuf[i]);
    }
    sendbuf[18]=Verify;
    memcpy(sendbuf+19,&WLP_TAIL,4);
    Usart1_SendData(sendbuf,23);
}

uint8_t Client_Receive()
{
    uint16_t i=0;
    uint8_t Verify=0x00;
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
    if(memcmp(MHID,(u1mbuf->pData),10)==0 || u1mbuf->pData[0]==0)//序列号相同    //如果是发给管理主机的信息
    {
        switch(*(u1mbuf->pData+10))
        {
            case 0x30: 
                Client_Rx30Tx31(); //设置命令
                break;
            case 0x32:
                Client_Rx32Tx33(); //读取设置信息
                break;
            case 0x34:
                Client_Rx34Tx35(); //配置仪器
                break;
            case 0x76:
                Client_Rx76Tx77(); //时间同步
                break;
            case 0x78:
                Client_Rx78Tx79(); //数据下载
                break;
            case 0x80:
                Client_Rx80Tx81(); //报警记录下载
                break;
            case 0x72:
                Client_Rx72Tx73(); //数据删除
                break;
            default:
                break;
        }
    }
//    else if(u1mbuf->pData[0]==0)  //序列号为0 重新写序列号
//    {
//        switch(*(u1mbuf->pData+10))
//        {
//            case 0xCC:

//                break;
//            default:
//                break;
//        }
//    }
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
