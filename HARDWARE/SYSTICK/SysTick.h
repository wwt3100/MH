#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f10x.h"

void SysTick_Init(void);
//void Delay_us(__IO u32 nTime); // �жϵķ�ʽ
void Delay_ms(__IO u32 nTime); // �жϵķ�ʽ
void TimingDelay_Decrement(void);

void timer_init(uint32_t* pnTimer, uint32_t nValue);
uint32_t timer_check(uint32_t nTimer);
uint32_t timer_check_nolimit(uint32_t nTimer);

#endif /* __SYSTICK_H */
