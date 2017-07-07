#ifndef __MH_STRUCT_H
#define __MH_STRUCT_H

#include <stm32f10x.h>                  // Device header

#pragma pack(4)

typedef enum 
{
    e_SeverMode_485RJ45=0,
    e_SeverMode_RJ45,
    e_ClientMode_RJ45,
    e_ClientMode_GPRS,
}e_GlobalConfig;

typedef enum 
{
    SST_ServerIDLE=0,
    SST_ServerRun,
    CST_ClientHasData,
    CST_ClientNoData,
}e_Stat;

typedef enum 
{
    e_Stat_Sampling,
    e_Stat_SampleingWait,
    e_Stat_Idle,
    e_Stat_PCMessage,
    e_Stat_PCMessageWait,
}__485ServerStat;

typedef struct HostStat
{
    uint8_t SDCardStat;
    uint8_t ServerStat;
    uint8_t ClientStat;
}_HostStat;

typedef struct GlobalConfig
{
//    uint8_t RunMode;
//    uint8_t SaveMode; //Save All , Save Monitor Device
//    uint8_t SeverMode; //  485/zigbee/RJ45
//    uint8_t ClientMode; // RJ45/GPRS
    
    uint8_t PhoneNumber[5][16];
    uint8_t OverLimitInterval;  //超限报警间隔
    uint8_t AlarmIntervalTime; //声光报警间隔时间 
    uint8_t OfflineAlarmInterval;  //掉线报警间隔  
    uint8_t OverLimitONOFF;
    uint8_t AlarmONOFF;
    uint8_t OfflineAlarmONOFF;
    uint8_t SMSAlarmNum;      //短信报警连发次数
    uint8_t SamplingInterval;  //采样间隔 单位 秒  固定60秒
    uint8_t MonitorDeviceNum; //监控的仪器数量
    
    uint16_t RecodeInterval;  //记录间隔 固定30分钟
    uint16_t RetryInterval;  //采样重试时间间隔 单位0.1s
    
}_GlobalConfig;

typedef enum
{
    e_Temp,
    
}e_DataType;

typedef struct DeviceConfig
{
    uint8_t ID[12]; //used 10byte
    uint8_t DeviceName[24]; 
    //uint8_t DataType[4];
    int32_t Data1Max;
    int32_t Data1Min;
    int32_t Data2Max;
    int32_t Data2Min;
}_DeviceConfig;

typedef struct DeviceData
{
    uint8_t ID[12];
    int32_t Data1;
    int32_t Data2;
    uint8_t time[8];
    uint8_t Alram[4];   //[0]->offline   [1]->data1  [2]->data2
    uint32_t OfflineAlarmTimer;
    uint32_t Data1AlarmTimer;
    uint32_t Data2AlarmTimer;
}_DeviceData;

//typedef struct AlarmLine
//{
//    uint8_t ID[12]; //used 10byte
//    uint8_t time[4];
//    int32_t Data1;
//    int32_t Data2;
//    int32_t Data1Max;
//    int32_t Data1Min;
//    int32_t Data2Max;
//    int32_t Data2Min;
//}_AlarmLine;

#endif
