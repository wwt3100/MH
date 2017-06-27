#ifndef __DATA_FILE_H
#define __DATA_FILE_H

#include "stm32f10x.h"                  // Device header
#include "MH-Struct.h"













void SaveData2RecodeFile(uint8_t* ID ,int32_t* Data1 ,int32_t Data2);
void SaveData2TempFile(_DeviceData *dd);


#endif
