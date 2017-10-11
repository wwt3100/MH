

#include "SMSAlarm.h"
#include "led.h"
extern _DeviceConfig cDc[255];
extern _DeviceData _Dd[255];
extern __mbuf *u1mbuf,*u2mbuf,*u3mbuf,*gmbuf;

char* SMSAlarm_GetLine(void);
void ASCII2UNICODE(char* str);
uint8_t *SMSAlarmMessage=0;
extern const _GlobalConfig c_gc;
extern volatile _GlobalConfig _gc;
extern volatile _HostStat hstat;

extern FATFS *fs;
extern struct rtc_time systmtime;
extern const char MHID[];
__abuf *abuf=0;
void SaveData2AlarmFile(uint8_t yn)
{
//    FATFS *fs;     /* Ponter to the filesystem object */
    uint32_t fres,wbt;
    DIR dj;         /* Directory search object */
    FILINFO *fno;    /* File information */
    FIL *fp;
    uint32_t t;
    if(SD_CardIsInserted())
    {
//        fs = malloc(1020);//malloc(sizeof (FATFS));
        fp = malloc(636);
        fno= malloc(380);
        fres=f_mount(fs, "0:", 0);
        if(fres==FR_OK)
        {
            fres=f_findfirst(&dj, fno, "", ".Alarmdata");
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
                    if(abuf->AlarmType == eAlarmType_PowerOff || abuf->AlarmType == eAlarmType_PowerOn)
                    {
                        f_write(fp,MHID,10,&wbt);
                        to_tm(RTC_GetCounter(), &systmtime);
                        t=TimeCompress(NULL); 
                    }
                    else
                    {
                        f_write(fp,cDc[abuf->dev].ID,10,&wbt);
                        t=TimeCompress(_Dd[abuf->dev].time);  //fix bug time ==0
                    }                    
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
//        f_mount(0,"0:",1);
//        free(fs);
        free(fp);
        free(fno);
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
                    memcpy(buf->DeviceName,cDc[dev].DeviceName,24);
                    memcpy(buf->PhoneNumber,(uint8_t*)_gc.PhoneNumber[i],16);
                    switch(type)
                    {
                        case eAlarmType_Online:
                        case eAlarmType_Offline:
                            to_tm(RTC_GetCounter(), &systmtime);
                            buf->time[0]=systmtime.tm_year-2000;
                            buf->time[1]=systmtime.tm_mon ;
                            buf->time[2]=systmtime.tm_mday;
                            buf->time[3]=systmtime.tm_hour;
                            buf->time[4]=systmtime.tm_min ;
                            buf->time[5]=systmtime.tm_sec ;
                            break;
                        case eAlarmType_OverLimitRecovery:
                        case eAlarmType_OverLimit:
                            memcpy(buf->time,_Dd[dev].time,8);
                            break;
                        default:
                            break;
                    }
                    
                    buf->Data1=_Dd[dev].Data1;
                    buf->Data2=_Dd[dev].Data2;
                    buf->Data1Max=cDc[dev].Data1Max;
                    buf->Data1Min=cDc[dev].Data1Min;
                    buf->Data2Max=cDc[dev].Data2Max;
                    buf->Data2Min=cDc[dev].Data2Min;
                    buf->usable=1;
                    buf->pNext=(__abuf*)CreateAlarmbuf(124);
                    buf=buf->pNext;
                }
            }
            break;
        case eAlarmType_PowerOff:
        case eAlarmType_PowerOn:  
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
                    memcpy(buf->PhoneNumber,(uint8_t*)_gc.PhoneNumber[i],16);
                    to_tm(RTC_GetCounter(),&systmtime);
                    buf->time[0]=systmtime.tm_year-2000;
                    buf->time[1]=systmtime.tm_mon ;
                    buf->time[2]=systmtime.tm_mday;
                    buf->time[3]=systmtime.tm_hour;
                    buf->time[4]=systmtime.tm_min ;
                    buf->time[5]=systmtime.tm_sec ;
                    buf->usable=1;
                    buf->pNext=(__abuf*)CreateAlarmbuf(124);
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
    static uint8_t a=0;
    static uint8_t dev=0;
    static uint32_t hasAlarm=0;
    if(dev==0)
    {
        AlarmOn=hasAlarm;
        hasAlarm=0;
    }
    if(c_gc.MonitorDeviceNum==0)
    {
        return 1;
    }

    if(_gc.OfflineAlarmONOFF)
    {
        if(timer_check((_Dd[dev].OfflineAlarmTimer)) && _Dd[dev].Alram[0]<_gc.SMSAlarmNum+1)
        {
            timer_init_sp(&(_Dd[dev].OfflineAlarmTimer),_gc.OfflineAlarmInterval*60000);
            _Dd[dev].Alram[0]+=1;
            SMSAlarm(eAlarmType_Offline,dev,0); //下线报警
        }
        if(_Dd[dev].Alram[0]>0 && _Dd[dev].OfflineAlarmTimer==0)
        {
            _Dd[dev].Alram[0]=0;
            SMSAlarm(eAlarmType_Online,dev,0);  // 设备上线恢复
        }
        hasAlarm+=_Dd[dev].Alram[0];
    }
    if(_gc.OverLimitONOFF)
    {
        if(timer_check((_Dd[dev].Data1AlarmTimer)) && _Dd[dev].Alram[1]<_gc.SMSAlarmNum+1)
        {
            timer_init_sp(&_Dd[dev].Data1AlarmTimer,_gc.OverLimitInterval*60000);
            if(_Dd[dev].Data1>cDc[dev].Data1Max || _Dd[dev].Data1<cDc[dev].Data1Min)  //消除偶发性不超限报警的情况
            {
                _Dd[dev].Alram[1]+=1; 
                SMSAlarm(eAlarmType_OverLimit,dev,1);  //超限报警
            }
        }
        if(_Dd[dev].Alram[1]>0 && _Dd[dev].Data1AlarmTimer==0) //使用nolimit timer可能为零  fixed:初始化使用_sp函数
        {
            _Dd[dev].Alram[1]=0;
            SMSAlarm(eAlarmType_OverLimitRecovery,dev,1); //超限报警 解除
        }
        hasAlarm+=_Dd[dev].Alram[1];
        
        if(timer_check((_Dd[dev].Data2AlarmTimer)) && _Dd[dev].Alram[2]<_gc.SMSAlarmNum+1)
        {
            timer_init_sp(&_Dd[dev].Data2AlarmTimer,_gc.OverLimitInterval*60000);
            if(_Dd[dev].Data2>cDc[dev].Data2Max || _Dd[dev].Data2<cDc[dev].Data2Min)
            {
                switch(_Dd[dev].ID[11])
                {
                    case 1:                    //温湿度
                        _Dd[dev].Alram[2]+=1; 
                        SMSAlarm(eAlarmType_OverLimit,dev,2); //超限报警
                        break;
                    case 3:                     //双温
                        _Dd[dev].Alram[2]+=1; 
                        SMSAlarm(eAlarmType_OverLimit,dev,1); //超限报警
                        break;
                    default:
                        break;
                }
            }
        }
        if(_Dd[dev].Alram[2]>0 && _Dd[dev].Data2AlarmTimer==0)
        {
            _Dd[dev].Alram[2]=0;
            switch(_Dd[dev].ID[11])
            {
                case 1:                    //温湿度
                    SMSAlarm(eAlarmType_OverLimitRecovery,dev,2); //超限报警 解除
                    break;
                case 3:                     //双温
                    SMSAlarm(eAlarmType_OverLimitRecovery,dev,1); //超限报警 解除
                    break;
                default:
                    break;
            }
        }
        hasAlarm+=_Dd[dev].Alram[2];
    }
    if(_gc.AlarmONOFF==1 && AlarmOn>0)
    {
        //LED3(1);
        //声光报警
        if(AlarmBellTimer==0)
        {
            AlarmBellTimer=1;
        }
    }
    else
    {
        LED3(Bit_RESET);
        AlarmBellTimer=0;
        GPIO_WriteBit(GPIOA,GPIO_Pin_4,Bit_RESET);
    }
    
    if(timer_check(AlarmBellTimer)) //报警灯闪烁  蜂鸣器间隔响
    {
        timer_init(&AlarmBellTimer,1000);
        //a=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);
        (a==0)?(a=1):(a=0);
        GPIO_WriteBit(GPIOA,GPIO_Pin_4,a);
        LED3(a);
    }
    dev++;
    if(_gc.MonitorDeviceNum<=dev)
    {
        dev=0;
    }
    return 0;
}
volatile uint16_t GSMWorkStat=0;
static uint16_t lastcmd=0;
uint32_t SMSAlarmTimeout=0;
void SMSAlarm_DoWork()
{
    __abuf *tb;
    uint8_t *sendbuf=0;
    char *str;//*str1;
    if(GSMWorkStat!=eGSMStat_Ready)
    {
        if(abuf->usable==1)
        {
            abuf->AlarmStat=eAlarmStat_SendError;
        }
        else
        {
            return;
        }
    }
    if(abuf->usable!=1)//|| GSMOK!=1)
        return;
    
//    str1=malloc(124);
//    memset(str1,0,124);
    switch(abuf->AlarmStat)
    {
        case eAlarmStat_Waiting:
            str=malloc(508);
            memset(str,0,508);
            sendbuf=malloc(1020);
            memset(sendbuf,0,1020);
            strcpy((char*)sendbuf,"AT+CSMP=17,167,0,8\r\nAT+CMGS=\"");
            strcpy(str,(char*)abuf->PhoneNumber);
            ASCII2UNICODE(str);
            strcat((char*)sendbuf,str);
            strcat((char*)sendbuf,"\"\n");
            sprintf(str,"20%02d-%02d-%02d %02d:%02d:%02d ",abuf->time[0],abuf->time[1],abuf->time[2],abuf->time[3],abuf->time[4],abuf->time[5]);
            //strcat((char*)sendbuf,str);
            
            
            switch(abuf->AlarmType)
            {
                case eAlarmType_OverLimit:
                    strcat((char*)str,(char*)abuf->DeviceName);
                    if(abuf->Option==1)
                    {
                        sprintf(str+strlen(str)," 温度超限:%d.%d ℃ (↓%d.%d,↑%d.%d)",abuf->Data1/10,abuf->Data1%10,abuf->Data1Min/10,abuf->Data1Min%10,abuf->Data1Max/10,abuf->Data1Max%10);
                    }
                    else if(abuf->Option==2)
                    {
                        sprintf(str+strlen(str)," 湿度超限:%d.%d %cRH (↓%d.%d,↑%d.%d)",abuf->Data2/10,abuf->Data2%10,'%',abuf->Data2Min/10,abuf->Data2Min%10,abuf->Data2Max/10,abuf->Data2Max%10);
                    }
                    break;
                case eAlarmType_OverLimitRecovery:
                    strcat((char*)str,(char*)abuf->DeviceName);
                    if(abuf->Option==1)
                    {
                        strcat(str," 温度恢复 超限报警解除!");
                    }
                    else if(abuf->Option==2)
                    {
                        strcat(str," 湿度恢复 超限报警解除!");
                    }
                    break;
                case eAlarmType_Offline:
                    strcat((char*)str,(char*)abuf->DeviceName);
                    strcat((char*)str," 设备掉线!");
                    break;
                case eAlarmType_Online:
                    strcat((char*)str,(char*)abuf->DeviceName);
                    strcat((char*)str," 设备上线恢复正常!");
                    break;
                case eAlarmType_PowerOff:
                    strcat((char*)str,MHID);
                    strcat((char*)str," HE2435管理主机断电!!!");
                    break;
                case eAlarmType_PowerOn:
                    strcat((char*)str,MHID);
                    strcat((char*)str," HE2435管理主机恢复供电!!!");
                    break;
                case eAlarmType_GoodStat:
                    strcat((char*)str,MHID);
                    strcat((char*)str," HE2435管理主机正在稳定运行!!!");
                    break;
                default:
                    break;
            }
            ASCII2UNICODE(str);  //
            sprintf(str+strlen(str),"%c",0x1a); //结束符
            strcat((char*)sendbuf,str);
            Usart2_SendData((char*)sendbuf,strlen((char*)sendbuf));
            lastcmd=eGCMD_CMGS;
            //send SMS
            abuf->AlarmStat=eAlarmStat_Sending;
            timer_init(&SMSAlarmTimeout,60000); 
            free(sendbuf);
            free(str);
            break;
        case eAlarmStat_Sending:
            // Wait
            if(timer_check(SMSAlarmTimeout)) //超时未发送成功 60s
            {
                abuf->AlarmStat=eAlarmStat_SendError;
            }
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
    
//    free(str1);
}


void SMSAlarm_GSMProcess()
{
    __mbuf* tb;
    char *cmd;
    if(u2mbuf->usable!=1)
        return;
    cmd=(char*)u2mbuf->pData;
//    if(cmd!=NULL)
    if(u2mbuf->datasize>2)
    {
        switch(*cmd)
        {
            case 'A':
                if(strncmp(cmd,"AT",2)==0)
                {
                    switch(*(cmd+2))
                    {
                        case 0x0d:
                            lastcmd=eGCMD_AT_R;
                            break;
                        case 'E':
                            lastcmd=eGCMD_ATE;
                            break;
                    }
                }
                break;
            case '+':
                if(strncmp(cmd,"+CMGS:",5)==0)
                {
                    lastcmd=eGCMD_CMGS;
                    //send sms ok
                    abuf->AlarmStat=eAlarmStat_SendOK;
                }
                if(strncmp(cmd,"+CMS ERROR",10)==0)
                {
                    switch(lastcmd)
                    {
                        case eGCMD_CMGS:
                            //send sms error
                            abuf->AlarmStat=eAlarmStat_SendError;
                            break;
                        default:
                            break;
                    }
                }
                if(strncmp(cmd,"+CMGF:0",5)==0)
                {
                    switch(*(cmd+7))
                    {
                        case '0':
                            lastcmd=eGCMD_CMGF_R_0;
                            GSMWorkStat=eGSMStat_Config;
                            break;
                        case '1':
                            lastcmd=eGCMD_CMGF_R_1;
                            GSMWorkStat=eGSMStat_Config2;
                            break;
                        default:
                            break;
                    }
                }
                if(strncmp(cmd,"+CPIN",5)==0)
                {
                    if(strstr(cmd,"READY")!=NULL)
                    {
                        GSMWorkStat=eGSMStat_CheckConfig;
                    }
                    else
                    {
                        GSMWorkStat=eGSMStat_NoSIMCard;
                    }
                }
                if(strncmp(cmd,"+CSCS",5)==0)
                {
                    if(strstr(cmd,"UCS2")!=NULL)
                    {
                        GSMWorkStat=eGSMStat_CheckConfig2;
                    }
                    else
                    {
                        GSMWorkStat=eGSMStat_CheckGSMREG;
                    }
                }
                if(strncmp(cmd,"+CREG",5)==0)
                {
                    switch(*(cmd+9))
                    {
                        case '1':
                        case '5':
                            GSMWorkStat=eGSMStat_Ready;
                            break;
                        case '2':
                            GSMWorkStat=eGSMStat_Wait;
                            break;
                        default:
                            GSMWorkStat=eGSMStat_NoSIMCard;
                            break;
                    }
                }
                break;
            case 'R':
                if(strncmp(cmd,"RING",4)==0)
                {
                    Usart2_SendData("ATH\r\n",5);
                }
                break;
//            case 'S':
//                if(strncmp(cmd,"SMS Ready",9)==0)
//                {
//                    GSMWorkStat=eGSMStat_Ready;
//                }
//                break;
            case 'O':
                if(strncmp(cmd,"OK",2)==0)
                {
                    switch(lastcmd)
                    {
                        case eGCMD_AT_R:
                            Usart2_SendData("ATE0&W\r\n",6);
                            lastcmd=eGCMD_ATE;
                            break;
                        case eGCMD_AT:
                            GSMWorkStat=eGSMStat_CheckSIMCARD;
                            break;
                        case eGCMD_AT_W:
                            GSMWorkStat=eGSMStat_CheckGSMREG;
                            break;
                        case eGCMD_CMGS:  //sms send ok
                            break;
                        case eGCMD_ATE:
                            GSMWorkStat=eGSMStat_CheckConfig;
                            break;
                        case eGCMD_CMGF_W:
                            GSMWorkStat=eGSMStat_CheckConfig2;
                            break;
                        case eGCMD_CFUN_0:
                            Usart2_SendData("AT+CFUN=1\r\n",11);
                            GSMWorkStat=eGSMStat_CFUN1;
                            lastcmd=eGCMD_CFUN_1;
                            break;
                        case eGCMD_CFUN_1:
                            //GSMWorkStat=eGSMStat_CheckSIMCARD;
                            break;
                        case eGCMD_CSCS_W:
                            Usart2_SendData("AT&W\r\n",6);
                            lastcmd=eGCMD_AT_W;
                            break;
                        default:
                            break;
                    }
                }
                break;
            case 'E':
                if(strncmp(cmd,"ERROR",5)==0)
                {
                    //resend
                    switch(lastcmd)
                    {
                        case eGCMD_CMGS:
                            abuf->AlarmStat=eAlarmStat_SendError;
                            GSMWorkStat=eGSMStat_CheckSIMCARD;
                            break;
                        case eGCMD_CPIN:
                        case eGCMD_CMGF_R_0:
                        case eGCMD_CREG_R:
                            GSMWorkStat=eGSMStat_NoSIMCard;
                            break;
                        case eGCMD_ATE:
                        case eGCMD_CMGF_W:
                            break;
                        default:
                            break;
                    }
                    //Usart2_ResendData();
                }
                break;
            default:
                break;
        }
        //free(cmd);
    }
    tb=u2mbuf->pNext;
    u2mbuf->usable=0;
    free(u2mbuf);
    u2mbuf=tb;
}
void SMSAlarm_GSMWorkStat()
{
    static uint32_t timeout=0;
    static uint8_t laststat=0;
    switch(GSMWorkStat)
    {
        case 0:
            GPIO_SetBits(GPIOC,GPIO_Pin_6); //上电
            timer_init(&timeout,5000); //延时5秒
            GSMWorkStat++;
            break;
        case 1:
            if(timer_check_nolimit(timeout))
            {
                Usart2_SendData("AT\r\n",4);
                lastcmd=eGCMD_AT;
                timer_init(&timeout,5000);
            }
            break;
        case eGSMStat_CheckSIMCARD:
            Usart2_SendData("AT+CPIN?\r\n",11);
            timer_init(&timeout,10000);
            lastcmd=eGCMD_CPIN;
            GSMWorkStat=eGSMStat_Wait;
            laststat=eGSMStat_CheckSIMCARD;
            break;
        case eGSMStat_CheckConfig:
            Usart2_SendData("AT+CMGF?\r\n",11);
            lastcmd=eGCMD_CMGF_R_0;
            timer_init(&timeout,10000);
            GSMWorkStat=eGSMStat_Wait;
            laststat=eGSMStat_CheckConfig;
            break;
        case eGSMStat_CheckConfig2:
            Usart2_SendData("AT+CSCS?\r\n",11);
            lastcmd=eGCMD_CSCS_R;
            timer_init(&timeout,10000);
            GSMWorkStat=eGSMStat_Wait;
            laststat=eGSMStat_CheckConfig2;
            break;
        case eGSMStat_Config:
            Usart2_SendData("AT+CMGF=1\r\n",11);
            lastcmd=eGCMD_CMGF_W;
            timer_init(&timeout,10000);
            GSMWorkStat=eGSMStat_Wait;
            laststat=eGSMStat_Config;
            break;
        case eGSMStat_Config2:
            Usart2_SendData("AT+CSCS=\"UCS2\"\r\n",16);
            lastcmd=eGCMD_CSCS_W;
            timer_init(&timeout,10000);
            GSMWorkStat=eGSMStat_Wait;
            laststat=eGSMStat_Config2;
            break;
        case eGSMStat_CheckGSMREG:
            Usart2_SendData("AT+CREG?\r\n",10);
            lastcmd=eGCMD_CREG_R;
            timer_init(&timeout,3000);
            GSMWorkStat=eGSMStat_Wait;
            laststat=eGSMStat_CheckGSMREG;
            break;
        case eGSMStat_NoSIMCard:
            if(timer_check_nolimit(timeout))
            {
            Usart2_SendData("AT+CFUN=0\r\n",11);
            lastcmd=eGCMD_CFUN_0;
            timer_init(&timeout,10000);
            GSMWorkStat=eGSMStat_Wait;
            laststat=eGSMStat_NoSIMCard;
            }
            break;
        case eGSMStat_Wait:
            if(timer_check_nolimit(timeout))
            {
                GSMWorkStat=laststat;
            }
            break;
        case eGSMStat_CFUN1:
            timer_init(&timeout,10000);
            GSMWorkStat++;
            break;
        case eGSMStat_CFUN1Wait:
            if(timer_check_nolimit(timeout))
            {
                GSMWorkStat=eGSMStat_CheckSIMCARD;
            }
            break;

        default: //GSM系统正常运行
            break;
    }
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

//char* SMSAlarm_GetLine()
//{
//    static const char endline[3]={0x0d,0x0a,0x00};
//    static uint8_t *ptr=NULL;
//    char *p=NULL,*pr=NULL;
//    if(ptr==NULL)
//        ptr=u2mbuf->pData;
//    p=strstr((char*)ptr,endline);
//    if(p!=NULL)
//    {
//        pr=malloc(60);
//        memset(pr,0,60);
//        memcpy(pr,ptr,(uint8_t*)p-ptr);
//        ptr=(uint8_t*)p+2;
//    }
//    return NULL;
//}

///////////////////////////////////////////////////////////////////
// ASCII转Unicode 
//  短信发送中文需要,输入字符串长度要小于252/4=63个字符
//  
static char orgst[256];
void ASCII2UNICODE(char* str)
{
    uint8_t i=0;
    char *pt,*po;
    WCHAR c;
    //orgst=malloc(252);
    strcpy(orgst,str);
    memset(str,0,252);
    pt=str;
    po=orgst;
    for(;i<strlen(orgst);i++)
    {
        c=*po++;
        if (dbc_1st((BYTE)c) && i != 8 && i != 11 && dbc_2nd(*po)) {
			c = c << 8 | *po;
            i++;
            po++;
		}
		c = ff_oem2uni(c, FF_CODE_PAGE);	/* OEM -> Unicode */
		if (!c) c = '?';
        sprintf(pt,"%04X",c);
        pt+=4;
    }
    memset(orgst,0,252);
}
