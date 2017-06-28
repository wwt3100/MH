#ifndef __DATA_FILE_H
#define __DATA_FILE_H

#include "stm32f10x.h"                  // Device header
#include "MH-Struct.h"













void SaveData2RecodeFile(_DeviceData *dd);
void SaveData2TempFile(_DeviceData *dd);


#endif
