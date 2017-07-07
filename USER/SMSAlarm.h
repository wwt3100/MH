#ifndef __SMS_ALARM_H
#define __SMS_ALARM_H


#include "stm32f10x.h"                  // Device header
#include "MH-Struct.h"
#include "ff.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdio_sdcard.h"
#include "rtc.h"
#include "SysTick.h"
#include "usart.h"

enum eAlarmStat
{
    eAlarmStat_SendError=0,
    eAlarmStat_SendOK,
    eAlarmStat_Waiting,
    eAlarmStat_Sending,
};

enum eAlarmType
{
    eAlarmType_OverLimit=1,
    eAlarmType_OverLimitRecovery,
    eAlarmType_Offline,
    eAlarmType_Online,
};


#pragma pack(2)
typedef struct TAlrmbuf	
{
    struct TAlrmbuf	*pNext;
    //uint8_t packed;
    uint8_t usable;//是否是可用buff
    uint8_t AlarmStat; //报警状态
    uint8_t AlarmType; //报警类型
    uint8_t Option; //其他选项
    uint8_t dev;
    uint8_t PhoneNumber[16];
    uint8_t time[8];
    int16_t Data1;
    int16_t Data2;
    int16_t Data1Max;
    int16_t Data1Min;
    int16_t Data2Max;
    int16_t Data2Min;
}__abuf;










uint8_t SMSAlarm(uint16_t type,uint16_t dev,uint8_t op);

void SMSAlarm_Process(void);


__abuf* CreateAlarmbuf(uint16_t length);
























#endif
