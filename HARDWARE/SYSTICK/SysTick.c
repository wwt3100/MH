/**********************************************************************************
 * 文件名  ：SysTick.c
 * 描述    ：
 *             
 * 库版本  ：ST3.0.0
**********************************************************************************/

#include "SysTick.h"
extern uint32_t g_tick_1ms;// 毫秒计数器
//static __IO u32 TimingDelay;

/****************************************************************************
 * 函数名：SysTick_Init
 * 描述  ：
 * 输入  ：无
 * 输出  ：无
 ***************************************************************************/
void SysTick_Init(void)
{  
  if (SysTick_Config(SystemCoreClock / 1000U))
  {     
      NVIC_SystemReset();
      while (1);			/* Capture error */ 
  }
} 

/****************************************************************************
 * 函数名：Delay_us
 * 描述  ：
 * 输入  ：- nTime
 * 输出  ：无
 * 调用  ：
 ***************************************************************************/

//void Delay_us(__IO u32 nTime)
//{ 
//  TimingDelay = nTime;

//  while(TimingDelay != 0);
//}

/****************************************************************************
 * 函数名：Delay_ms
 * 描述  ：
 * 输入  ：- nTime
 * 输出  ：无
 * 调用  ：
 ***************************************************************************/ 
void Delay_ms(__IO u32 nTime)
{ 
    uint32_t timer;
    timer_init(&timer,nTime);
    while(timer_check(timer) == 0);
}


/****************************************************************************
 * 函数名：TimingDelay_Decrement
 * 描述  ：获取节拍程序
 * 输入  ：无
 * 输出  ：无
 * 调用  ：在 SysTick 中断函数 SysTick_Handler()调用
 ***************************************************************************/ 
//void TimingDelay_Decrement(void)
//{
//  if (TimingDelay != 0x00)
//  { 
//    TimingDelay--;
//  }
//}
 
/**----------------------------------------------------------------------------
 * 初始化计数器终值
 * 
 * @author xwm
 * @date   2015.11.20
 * 
 * @param  pnTimer 计数器指针
 * @param  nValue  值
 * @note   
 *----------------------------------------------------------------------------*/
void timer_init(uint32_t* pnTimer, uint32_t nValue)
{
    if (nValue == 0)
    {
        *pnTimer = 0;
    }
    else
    {
        *pnTimer = g_tick_1ms + nValue;
        if (*pnTimer == 0)
        {
            (*pnTimer)++;
        }
    }
}
void timer_init_sp(uint32_t* pnTimer, uint32_t nValue)
{
    if (nValue == 0)
    {
        *pnTimer = 1;
    }
    else
    {
        *pnTimer = g_tick_1ms + nValue;
        if (*pnTimer == 0)
        {
            (*pnTimer)++;
        }
    }
}
/**----------------------------------------------------------------------------
 * 检查计数器是否已经到达终值
 * 
 * @author xwm
 * @date   2015.11.20
 * 
 * @param  nTimer 计数器
 * 
 * @return int  0未使用或未到达 1到达终值
 * @note   计数器被人为赋值0时,该计数器为未使用状态
 *----------------------------------------------------------------------------*/
uint32_t timer_check(uint32_t nTimer)
{
    if (nTimer != 0 && (int32_t)(g_tick_1ms - nTimer) >= 0)
    {
        return g_tick_1ms - nTimer;   // true
    }
    else
    {
        return 0;   // false
    }
}
uint32_t timer_check_nolimit(uint32_t nTimer)
{
    if ((int32_t)(g_tick_1ms - nTimer) >= 0)
    {
        return g_tick_1ms - nTimer;   // true
    }
    else
    {
        return 0;   // false
    }
}

