

#include "SMSAlarm.h"
#include "led.h"
extern _DeviceConfig cDc[255];
extern _DeviceData _Dd[255];


uint8_t *SMSAlarmMessage=0;

extern volatile _GlobalConfig _gc;
extern volatile _HostStat hstat;

__abuf *abuf=0;
void SaveData2AlarmFile(uint8_t yn)
{
    FATFS *fs;     /* Ponter to the filesystem object */
    uint32_t fres,wbt;
    DIR dj;         /* Directory search object */
    FILINFO fno;    /* File information */
    FIL *fp;
    uint32_t t;
    if(SD_CardIsInserted())
    {
        fs = malloc(636);//malloc(sizeof (FATFS));
        fp = malloc(636);
        fres=f_mount(fs, "0:", 0);
        if(fres==FR_OK)
        {
            fres=f_findfirst(&dj, &fno, "", ".Alarmdata");
            switch(fres)
            {
                case FR_NO_FILE:
                    //f_open(&fp,".Alarmdata",FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
                    //f_close(&fp);
                    //f_chmod(".Alarmdata",AM_ARC|AM_HID,AM_ARC|AM_HID); 
                case FR_OK:
                    f_open(fp,".Alarmdata",FA_OPEN_APPEND | FA_WRITE | FA_READ);
                    f_write(fp,&yn,1,&wbt);  //成功失败
                    f_write(fp,&abuf->AlarmType,1,&wbt);
                    f_write(fp,abuf->PhoneNumber,16,&wbt);
                    f_write(fp,cDc[abuf->dev].ID,10,&wbt);
                    t=TimeCompress(_Dd[abuf->dev].time);  //fix bug time ==0
                    f_write(fp,&t,4,&wbt);
                    f_write(fp,&_Dd[abuf->dev].Data1,2,&wbt);
                    f_write(fp,&cDc[abuf->dev].Data1Max,2,&wbt);
                    f_write(fp,&cDc[abuf->dev].Data1Min,2,&wbt);
                    f_write(fp,&_Dd[abuf->dev].Data2,2,&wbt);
                    f_write(fp,&cDc[abuf->dev].Data2Max,2,&wbt);
                    f_write(fp,&cDc[abuf->dev].Data2Min,2,&wbt);
                    f_close(fp);
                    break;
                default:
                    break;
            }
        }
        f_mount(0,"0:",0);
        free(fs);
        free(fp);
    }
}


uint8_t SMSAlarm(uint16_t type,uint16_t dev,uint8_t op)
{
    __abuf *buf=abuf;
    uint8_t i=0;
    switch(type)
    {
        case eAlarmType_Online:
        case eAlarmType_OverLimitRecovery:
        case eAlarmType_Offline:
        case eAlarmType_OverLimit:
            while(buf->pNext!=NULL)
            {
                buf=buf->pNext;
            }
            for(i=0;i<5;i++)
            {
                if(_gc.PhoneNumber[i][0]!=0)
                {
                    buf->AlarmStat=eAlarmStat_Waiting;
                    buf->AlarmType=type;
                    buf->Option=op;
                    buf->dev=dev;
                    memcpy(buf->PhoneNumber,(uint8_t*)_gc.PhoneNumber[i],16);
                    memcpy(buf->time,_Dd[dev].time,8);
                    buf->Data1=_Dd[dev].Data1;
                    buf->Data2=_Dd[dev].Data2;
                    buf->Data1Max=cDc[dev].Data1Max;
                    buf->Data1Min=cDc[dev].Data1Min;
                    buf->Data2Max=cDc[dev].Data2Max;
                    buf->Data2Min=cDc[dev].Data2Min;
                    buf->usable=1;
                    buf->pNext=(__abuf*)CreateAlarmbuf(60);
                    buf=buf->pNext;
                }
            }
            break;
        default:
            return 1;
    }
    
    
    
    

    return 0;
}
uint32_t AlarmOn=0;
static uint32_t AlarmBellTimer=0;
uint8_t SMSAlarm_SetBuf()
{
    uint8_t a=0;
    static uint8_t dev=0;
    if(_gc.OfflineAlarmONOFF)
    {
        if(timer_check((_Dd[dev].OfflineAlarmTimer)) && _Dd[dev].Alram[0]==0)
        {
            //timer_init_sp(&(_Dd[dev].OfflineAlarmTimer),_gc.OfflineAlarmInterval*60000);
            _Dd[dev].Alram[0]=1;
            for(a=0;a<_gc.SMSAlarmNum;a++)
            {
                SMSAlarm(eAlarmType_Offline,dev,0); //下线报警
            }
            AlarmOn++;
        }
        if(_Dd[dev].Alram[0]>0 && _Dd[dev].OfflineAlarmTimer==0)
        {
            _Dd[dev].Alram[0]=0;
            for(a=0;a<_gc.SMSAlarmNum;a++)
            {
                SMSAlarm(eAlarmType_Online,dev,0);  // 设备上线恢复
            }
            AlarmOn--;
        }
    }
    if(_gc.OverLimitONOFF)
    {
        if(timer_check((_Dd[dev].Data1AlarmTimer)) && _Dd[dev].Alram[1]==0)
        {
            //timer_init_sp(&_Dd[dev].Data1AlarmTimer,_gc.AlarmIntervalTime*60000);
            _Dd[dev].Alram[1]=1; 
            for(a=0;a<_gc.SMSAlarmNum;a++)
            {
                SMSAlarm(eAlarmType_OverLimit,dev,1);  //超限报警
            }
            AlarmOn++;
        }
        if(_Dd[dev].Alram[1]>0 && _Dd[dev].Data1AlarmTimer==0) //使用nolimit timer可能为零  fixed:初始化使用_sp函数
        {
            _Dd[dev].Alram[1]=0;
            AlarmOn--;
            for(a=0;a<_gc.SMSAlarmNum;a++)
            {
                SMSAlarm(eAlarmType_OverLimitRecovery,dev,1); //超限报警 解除
            }
        }
        if(timer_check((_Dd[dev].Data2AlarmTimer)) && _Dd[dev].Alram[2]==0)
        {
            //timer_init_sp(&_Dd[dev].Data2AlarmTimer,_gc.AlarmIntervalTime*60000);
            _Dd[dev].Alram[2]=1; 
            for(a=0;a<_gc.SMSAlarmNum;a++)
            {
                SMSAlarm(eAlarmType_OverLimit,dev,2); //超限报警
            }
            AlarmOn++;
        }
        if(_Dd[dev].Alram[2]>0 && _Dd[dev].Data2AlarmTimer==0)
        {
            _Dd[dev].Alram[2]=0;
            AlarmOn--;
            for(a=0;a<_gc.SMSAlarmNum;a++)
            {
                SMSAlarm(eAlarmType_OverLimitRecovery,dev,2); //超限报警 解除
            }
        }
    }
    if(_gc.AlarmONOFF==1 && AlarmOn>0)
    {
        LED3(1);
        //声光报警
        if(AlarmBellTimer==0)
        {
            AlarmBellTimer=1;
        }
    }
    else
    {
        LED3(0);
        AlarmBellTimer=0;
        GPIO_WriteBit(GPIOA,GPIO_Pin_4,0);
    }
    
    if(timer_check(AlarmBellTimer)) //蜂鸣器间隔响
    {
        timer_init(&AlarmBellTimer,1000);
        a=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);
        (a==0)?(a=1):(a=0);
        GPIO_WriteBit(GPIOA,GPIO_Pin_4,a);
    }
    dev++;
    if(_gc.MonitorDeviceNum<=dev)
    {
        dev=0;
    }
    return 0;
}
extern uint8_t GSMOK;
void SMSAlarm_DoWork()
{
    __abuf *tb;
    uint8_t *sendbuf=0;
    char *str;
    if(abuf->usable!=1)//|| GSMOK!=1)
        return;
    str=malloc(124);
    memset(str,0,124);
    switch(abuf->AlarmStat)
    {
        case eAlarmStat_Waiting:
            sendbuf=malloc(252);
            strcpy((char*)sendbuf,"ZZZZZ#AT+SMSEND=\"");
            strcat((char*)sendbuf,(char*)abuf->PhoneNumber);
            strcat((char*)sendbuf,"\",3,\"");
            sprintf(str,"20%02d-%02d-%02d %02d:%02d:%02d ",abuf->time[0],abuf->time[1],abuf->time[2],abuf->time[3],abuf->time[4],abuf->time[5]);
            strcat((char*)sendbuf,str);
            strcat((char*)sendbuf,(char*)cDc[abuf->dev].DeviceName);
            
            switch(abuf->AlarmType)
            {
                case eAlarmType_OverLimit:
                    if(abuf->Option==1)
                    {
                        sprintf(str,"温度超限:%d.%d ℃ (↓%d.%d,↑%d.%d)",abuf->Data1/10,abuf->Data1%10,abuf->Data1Min/10,abuf->Data1Min%10,abuf->Data1Max/10,abuf->Data1Max%10);
                        strcat((char*)sendbuf,str);
                    }
                    else if(abuf->Option==2)
                    {
                        sprintf(str,"湿度超限:%d.%d %cRH (↓%d.%d,↑%d.%d)",abuf->Data2/10,abuf->Data2%10,'%',abuf->Data2Min/10,abuf->Data2Min%10,abuf->Data2Max/10,abuf->Data2Max%10);
                        strcat((char*)sendbuf,str);
                    }
                    break;
                case eAlarmType_OverLimitRecovery:
                    if(abuf->Option==1)
                    {
                        sprintf(str,"温度恢复 超限报警解除!");
                        strcat((char*)sendbuf,str);
                    }
                    else if(abuf->Option==2)
                    {
                        sprintf(str,"湿度恢复 超限报警解除!");
                        strcat((char*)sendbuf,str);
                    }
                    break;
                case eAlarmType_Offline:
                    strcat((char*)sendbuf," 设备掉线");
                    break;
                case eAlarmType_Online:
                    strcat((char*)sendbuf," 设备上线恢复正常!");
                    break;
                default:
                    break;
            }
            strcat((char*)sendbuf,"\"\r\n");
            Usart2_SendData(sendbuf,strlen((char*)sendbuf));
            //send SMS
            abuf->AlarmStat=eAlarmStat_Sending;
            free(sendbuf);
            break;
        case eAlarmStat_Sending:
            // Wait
            break;
        case eAlarmStat_SendOK:
        case eAlarmStat_SendError:
            //write data
            SaveData2AlarmFile(abuf->AlarmStat);
            //clean buffer
            tb=abuf->pNext;
            abuf->usable=0;
            free(abuf);
            abuf=tb;
        default:
            break;
    }
    free(str);
}


void SMSAlarm_Process(void)
{
    SMSAlarm_SetBuf();
    SMSAlarm_DoWork();
}



__abuf* CreateAlarmbuf(uint16_t length)
{
    __abuf *p=NULL;
    p = (__abuf*)malloc(length);// + sizeof(__mbuf));
    if(p==NULL)
        return p;
    memset(p,0,length);
    
    return p;
}


