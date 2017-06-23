#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#define __ASM            __asm                                      /*!< asm keyword for ARM Compiler */
#define __INLINE         __inline                                   /*!< inline keyword for ARM Compiler */
#define __STATIC_INLINE  static __inline

void LED_Init(void);//³õÊ¼»¯


__STATIC_INLINE void LED1(BitAction onoff)
{
    GPIO_WriteBit(GPIOB,GPIO_Pin_12,onoff);
}
__STATIC_INLINE void LED2(BitAction onoff)
{
    GPIO_WriteBit(GPIOB,GPIO_Pin_13,onoff);
}
__STATIC_INLINE void LED3(BitAction onoff)
{
    GPIO_WriteBit(GPIOB,GPIO_Pin_14,onoff);
}	 	

#endif
