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
    uint8_t RunMode;
    uint8_t SaveMode; //Save All , Save Monitor Device
    uint8_t SeverMode; //  485/zigbee/RJ45
    uint8_t ClientMode; // RJ45/GPRS
    uint16_t MonitorDeviceNum; //��ص���������
    uint8_t PhoneNumber1[16];
    uint8_t PhoneNumber2[16];
    uint8_t PhoneNumber3[16];
    uint8_t PhoneNumber4[16];
    uint8_t PhoneNumber5[16];
    uint8_t OverLimitInterval;  //���ޱ������
    uint8_t OfflineAlarmInterval;  //���߱������
    uint8_t AlarmIntervalTime; //���ⱨ�����ʱ�� 
    uint8_t OverLimitONOFF;
    uint8_t OfflineAlarmONOFF;
    uint8_t AlarmONOFF;
    uint16_t SamplingInterval;  //������� ��λ ��  �̶�60��
    uint16_t RecodeInterval;  //��¼��� �̶�60��
    
    uint16_t RetryInterval;  //��������ʱ���� ��λ0.1s
    
}_GlobalConfig;

typedef enum
{
    e_Temp,
    
}e_DataType;

typedef struct DeviceConfig
{
    uint8_t ID[12]; //used 10byte
    uint8_t DeviceName[32]; 
    uint8_t DataType[4];
    int32_t Data1Max;
    int32_t Data1Min;
    int32_t Data2Max;
    int32_t Data2Min;
    int32_t Data3Max;
    int32_t Data3Min;
    int32_t Data4Max;
    int32_t Data4Min;
}_DeviceConfig;

typedef struct DeviceData
{
    uint8_t ID[12];
    int32_t Data1;
    int32_t Data2;
    int32_t Data3;
    int32_t Data4;
    uint8_t time[8];
    uint8_t Alram[4];
    uint32_t Data1AlarmTimer;
    uint32_t Data2AlarmTimer;
    uint32_t Data3AlarmTimer;
    uint32_t Data4AlarmTimer;
}_DeviceData;

#endif
