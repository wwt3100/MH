

#include "SMSAlarm.h"
#include "led.h"
extern _DeviceConfig cDc[255];
extern _DeviceData _Dd[255];

char* SMSAlarm_GetLine(void);
void ASCII2UNICODE(char* str)
uint8_t *SMSAlarmMessage=0;

extern volatile _GlobalConfig _gc;
extern volatile _HostStat hstat;

extern FATFS *fs;
extern struct rtc_time systmtime;

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
                    f_write(fp,&yn,1,&wbt);  //�ɹ�ʧ��
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
        case eAlarmType_PowerOff:
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
                    buf->pNext=(__abuf*)CreateAlarmbuf(60);
                    buf=buf->pNext;
                }
            }
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
                SMSAlarm(eAlarmType_Offline,dev,0); //���߱���
            }
            AlarmOn++;
        }
        if(_Dd[dev].Alram[0]>0 && _Dd[dev].OfflineAlarmTimer==0)
        {
            _Dd[dev].Alram[0]=0;
            for(a=0;a<_gc.SMSAlarmNum;a++)
            {
                SMSAlarm(eAlarmType_Online,dev,0);  // �豸���߻ָ�
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
                SMSAlarm(eAlarmType_OverLimit,dev,1);  //���ޱ���
            }
            AlarmOn++;
        }
        if(_Dd[dev].Alram[1]>0 && _Dd[dev].Data1AlarmTimer==0) //ʹ��nolimit timer����Ϊ��  fixed:��ʼ��ʹ��_sp����
        {
            _Dd[dev].Alram[1]=0;
            AlarmOn--;
            for(a=0;a<_gc.SMSAlarmNum;a++)
            {
                SMSAlarm(eAlarmType_OverLimitRecovery,dev,1); //���ޱ��� ���
            }
        }
        if(timer_check((_Dd[dev].Data2AlarmTimer)) && _Dd[dev].Alram[2]==0)
        {
            //timer_init_sp(&_Dd[dev].Data2AlarmTimer,_gc.AlarmIntervalTime*60000);
            _Dd[dev].Alram[2]=1; 
            for(a=0;a<_gc.SMSAlarmNum;a++)
            {
                SMSAlarm(eAlarmType_OverLimit,dev,2); //���ޱ���
            }
            AlarmOn++;
        }
        if(_Dd[dev].Alram[2]>0 && _Dd[dev].Data2AlarmTimer==0)
        {
            _Dd[dev].Alram[2]=0;
            AlarmOn--;
            for(a=0;a<_gc.SMSAlarmNum;a++)
            {
                SMSAlarm(eAlarmType_OverLimitRecovery,dev,2); //���ޱ��� ���
            }
        }
    }
    if(_gc.AlarmONOFF==1 && AlarmOn>0)
    {
        //LED3(1);
        //���ⱨ��
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
    
    if(timer_check(AlarmBellTimer)) //��������˸  �����������
    {
        timer_init(&AlarmBellTimer,1000);
        a=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);
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
extern uint8_t GSMOK;
uint32_t SMSAlarmTimeout=0;
void SMSAlarm_DoWork()
{
    __abuf *tb;
    uint8_t *sendbuf=0;
    char *str,*str1;
    if(abuf->usable!=1)//|| GSMOK!=1)
        return;
    str=malloc(124);
    memset(str,0,124);
    str1=malloc(124);
    memset(str1,0,124);
    switch(abuf->AlarmStat)
    {
        case eAlarmStat_Waiting:
            sendbuf=malloc(124);
            strcpy((char*)sendbuf,"AT+CMGS=\"");
            strcat((char*)sendbuf,(char*)abuf->PhoneNumber);
            strcat((char*)sendbuf,"\"\r");
            sprintf(str,"20%02d-%02d-%02d %02d:%02d:%02d ",abuf->time[0],abuf->time[1],abuf->time[2],abuf->time[3],abuf->time[4],abuf->time[5]);
            //strcat((char*)sendbuf,str);
            strcat((char*)str,(char*)cDc[abuf->dev].DeviceName);
            
            switch(abuf->AlarmType)
            {
                case eAlarmType_OverLimit:
                    if(abuf->Option==1)
                    {
                        sprintf(str+strlen(str)," �¶ȳ���:%d.%d �� (��%d.%d,��%d.%d)",abuf->Data1/10,abuf->Data1%10,abuf->Data1Min/10,abuf->Data1Min%10,abuf->Data1Max/10,abuf->Data1Max%10);
                    }
                    else if(abuf->Option==2)
                    {
                        sprintf(str+strlen(str)," ʪ�ȳ���:%d.%d %cRH (��%d.%d,��%d.%d)",abuf->Data2/10,abuf->Data2%10,'%',abuf->Data2Min/10,abuf->Data2Min%10,abuf->Data2Max/10,abuf->Data2Max%10);
                    }
                    break;
                case eAlarmType_OverLimitRecovery:
                    if(abuf->Option==1)
                    {
                        strcat(str," �¶Ȼָ� ���ޱ������!");
                    }
                    else if(abuf->Option==2)
                    {
                        strcat(str," ʪ�Ȼָ� ���ޱ������!");
                    }
                    break;
                case eAlarmType_Offline:
                    strcat((char*)str," �豸����");
                    break;
                case eAlarmType_Online:
                    strcat((char*)str," �豸���߻ָ�����!");
                    break;
                default:
                    break;
            }
            ASCII2UNICODE(str);  //
            sprintf(str+strlen(str),"%c",0x1a); //������
            strcat((char*)str,str1);
            Usart2_SendData(sendbuf,strlen((char*)sendbuf));
            //send SMS
            abuf->AlarmStat=eAlarmStat_Sending;
            timer_init(&SMSAlarmTimeout,60000); 
            free(sendbuf);
            break;
        case eAlarmStat_Sending:
            // Wait
            if(timer_check(SMSAlarmTimeout)) //��ʱδ���ͳɹ� 60s
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
    free(str);
    free(str1);
}
static uint16_t GSMWorkStat=0;
static uint16_t lastcmd=0;
void SMSAlarm_GSMProcess()
{
    char *cmd;
    cmd=SMSAlarm_GetLine();
    if(cmd!=NULL)
    {
        switch(*cmd)
        {
            case 'A':
                if(strcmp(cmd,"AT")==0)
                {
                    lastcmd=eGCMD_AT;
                }
                if(strcmp(cmd,"ATE0")==0)
                {
                    lastcmd=eGCMD_ATE;
                }
                break;
            case '+':
                if(strncmp(cmd,"+CMGS:",6)==0)
                {
                    lastcmd=eGCMD_CMGS;
                    //send sms ok
                    abuf->AlarmStat=eAlarmStat_SendOK;
                }
                if(strncmp(cmd,"+CMS ERROR",10)==0)
                {
                    lastcmd=eGCMD_CMGS;
                    //send sms error
                    abuf->AlarmStat=eAlarmStat_SendError;
                }
                if(strncmp(cmd,"+CMGF:0",7)==0)
                {
                    lastcmd=eGCMD_CMGF_R;
                }
                if(strncmp(cmd,"+CME ERROR:",11)==0)
                {
                    GSMWorkStat=99;
                }
                break;
            case 'R':
                if(strcmp(cmd,"RING")==0)
                {
                    Usart2_SendData("ATH\r\n",5);
                }
                break;
            case 'O':
                if(strcmp(cmd,"OK")==0)
                {
                    switch(lastcmd)
                    {
                        case eGCMD_AT:
                            Usart2_SendData("ATE0&W\r\n",6);
                            lastcmd=eGCMD_ATE;
                            break;
                        case eGCMD_CMGS:  //sms send ok
                            break;
                        case eGCMD_CMGF_R:
                            Usart2_SendData("AT+CMGF=1\r\n",11);
                            break;
                        case eGCMD_ATE:
                        case eGCMD_CMGF_W:
                            Usart2_SendData("AT&W\r\n",6);
                            lastcmd=eGCMD_AT_W;
                            break;
                        case eGCMD_CFUN:
                            Usart2_SendData("AT+CFUN=1\r\n",11);
                            GSMWorkStat=0;
                            break;
                        default:
                            break;
                    }
                }
                break;
            case 'E':
                if(strcmp(cmd,"ERROR")==0)
                {
                    //resend
                    switch(lastcmd)
                    {
                        case eGCMD_AT:
                            GSMWorkStat=99; //��������ϵ�����
                            break;
                        case eGCMD_CMGF_R:
                            Usart2_SendData("AT+CFUN=0\r\n",11);
                            lastcmd=eGCMD_CFUN;
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
        free(cmd);
    }
}
void SMSAlarm_GSMWorkStat()
{
    static uint32_t timeout=0;
    switch(GSMWorkStat)
    {
        case 0:
            if(timer_check_nolimit(timeout))
            {
                Usart2_SendData("AT\r\n",4);
                timer_init(&timeout,10000);
            }
            break;
        case 1:
            if(timer_check_nolimit(timeout))
            {
                Usart1_SendData("AT+CMGF=?\r\n",11);
                timer_init(&timeout,10000);
            }
            break;
        case 60:
            timer_init(&timeout,2000);
            GSMWorkStat=1;
            break;
        case 99:
            GPIO_ResetBits(GPIOC,GPIO_Pin_6); //GSM�ϵ�
            timer_init(&timeout,2000);
            GSMWorkStat=100;
            break;
        case 100:
            if(timer_check_nolimit(timeout))
            {
                GPIO_SetBits(GPIOC,GPIO_Pin_6); //�ϵ�
                timer_init(&timeout,500);
            }
            break;
        default: //GSMϵͳ��������
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

char* SMSAlarm_GetLine()
{

    return NULL;
}
void ASCII2UNICODE(char* str)
{
    char* orgst;
    orgst=malloc(124);
    strcpy(orgst,str);
    memset(str,0,124);
    //    if (dbc_1st((BYTE)c) && i != 8 && i != 11 && dbc_2nd(dp->dir[i])) {
//			c = c << 8 | dp->dir[i++];
//		}
//		c = ff_oem2uni(c, FF_CODE_PAGE);	/* OEM -> Unicode */
//		if (!c) c = '?';
    free(orgst);
}
