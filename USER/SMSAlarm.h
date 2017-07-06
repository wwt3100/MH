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




uint8_t SMSAlarm(void);
uint8_t SMSAlarm_Process(void);



























#endif
