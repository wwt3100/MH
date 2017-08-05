#ifndef __DATA_FILE_H
#define __DATA_FILE_H

#include "stm32f10x.h"                  // Device header
#include "MH-Struct.h"
#include "ff.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdio_sdcard.h"
#include "rtc.h"


enum DataFileStat
{
    eSDStat_NotOpen=0,
    eSDStat_Write,
};









void SaveData2RecodeFile(_DeviceData *dd);
void SaveData2TempFile(_DeviceData *dd);
FRESULT ReadTempFileSize(FSIZE_t *size);

#endif
