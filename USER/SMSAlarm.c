

#include "SMSAlarm.h"

extern _DeviceConfig cDc[255];
extern _DeviceData _Dd[255];


uint8_t *SMSAlarmMessage=0;

extern volatile _GlobalConfig _gc;
extern volatile _HostStat hstat;


void SaveData2AlarmFile(uint8_t device,uint8_t phonenum,uint8_t type)
{
    FATFS *fs;     /* Ponter to the filesystem object */
    uint32_t fres,wbt;
    DIR dj;         /* Directory search object */
    FILINFO fno;    /* File information */
    FIL fp;
    uint32_t t;
    if(SD_CardIsInserted())
    {
        fs = malloc(sizeof (FATFS));
        fres=f_mount(fs, "0:", 0);
        if(fres==FR_OK)
        {
            fres=f_findfirst(&dj, &fno, "", ".Alarmdata");
            switch(fres)
            {
                case FR_NO_FILE:
                    f_open(&fp,".Alarmdata",FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
                    f_close(&fp);
                    f_chmod(".Alarmdata",AM_ARC|AM_HID,AM_ARC|AM_HID); 
                case FR_OK:
                    f_open(&fp,".Alarmdata",FA_OPEN_APPEND | FA_WRITE | FA_READ);
                    f_write(&fp,&type,1,&wbt);
                    t=TimeCompress(_Dd->time);
                    f_write(&fp,&t,4,&wbt);
//                    f_write(&fp,&dd->Data1,2,&wbt);
//                    f_write(&fp,&dd->Data2,2,&wbt);
                    f_close(&fp);
                    break;
                default:
                    break;
            }
        }
        f_mount(0,"0:",0);
        free(fs);
    }
}


uint8_t SMSAlarm()
{
    char *Head={"ZZZZ#AT+SMSEND="};
    char *Sendbuf;
    if(SMSAlarmMessage==NULL)
    {
        return 1;
    }
    
    
    
    
    free(SMSAlarmMessage);
    SMSAlarmMessage=NULL;
    return 0;
}

uint8_t SMSAlarmPress()
{
    static uint8_t dev=0;
    if(timer_check((_Dd[dev].OfflineAlarmTimer)) && _Dd[dev].Alram[0]<_gc.SMSAlarmNum)
    {
        timer_init(&(_Dd[dev].OfflineAlarmTimer),_gc.OfflineAlarmInterval*60000);
        _Dd[dev].Alram[0]+=1; //下线报警
    }
    if(_Dd[dev].Alram[0]>1 && _Dd[dev].OfflineAlarmTimer==0)
    {
        _Dd[dev].Alram[0]=0;
        // 设备上线恢复
    }
    if(timer_check((_Dd[dev].Data1AlarmTimer)) && _Dd[dev].Alram[1]<_gc.SMSAlarmNum)
    {
        timer_init(&_Dd[dev].Data1AlarmTimer,_gc.AlarmIntervalTime*60000);
        _Dd[dev].Alram[1]+=1; //超限报警
    }
    if(_Dd[dev].Alram[1]>1 && _Dd[dev].Data1AlarmTimer==0)
    {
        _Dd[dev].Alram[1]=0;
        //超限报警 解除
    }
    if(timer_check((_Dd[dev].Data2AlarmTimer)) && _Dd[dev].Alram[2]<_gc.SMSAlarmNum)
    {
        timer_init(&_Dd[dev].Data2AlarmTimer,_gc.AlarmIntervalTime*60000);
        _Dd[dev].Alram[2]+=1; //超限报警
    }
    if(_Dd[dev].Alram[2]>1 && _Dd[dev].Data2AlarmTimer==0)
    {
        _Dd[dev].Alram[2]=0;
        //超限报警 解除
    }
    
    
    if(_gc.MonitorDeviceNum<=++dev)
    {
        dev=0;
    }
}








