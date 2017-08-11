
#ifndef __485SERVER_H
#define __485SERVER_H

#include "stm32f10x.h"  



uint8_t Server_Process(void);
uint8_t Server_Process_Li(void);
__inline static void __485SetReceive()
{
    GPIO_ResetBits(GPIOD,GPIO_Pin_8);
}

__inline static void __485SetSend()
{
    GPIO_SetBits(GPIOD,GPIO_Pin_8);
}





























#endif
