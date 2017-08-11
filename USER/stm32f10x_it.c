/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h" 
#include <stdio.h>
#include "mbuf.h"
#include "SMSAlarm.h"
 
void NMI_Handler(void)
{
}
 
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
#ifndef _DEBUG    
  NVIC_SystemReset();
#else
  while (1)
  {
  }
#endif
}
 
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
#ifndef _DEBUG    
  NVIC_SystemReset();
#else
  while (1)
  {
  }
#endif
}

 
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
#ifndef _DEBUG    
  NVIC_SystemReset();
#else
  while (1)
  {
  }
#endif
}
 
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
#ifndef _DEBUG    
  NVIC_SystemReset();
#else
  while (1)
  {
  }
#endif
}
 
void SVC_Handler(void)
{
}
 
void DebugMon_Handler(void)
{
}
 
void PendSV_Handler(void)
{
}
 
extern volatile uint32_t g_tick_1ms;
void SysTick_Handler(void)
{
	g_tick_1ms++;
}

void RTC_IRQHandler(void)
{
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
  {	     
    RTC_ClearITPendingBit(RTC_IT_SEC);		/* Clear the RTC Second interrupt */ 
	//Updata_1sec(&Timer); 
    RTC_WaitForLastTask();					/* Wait until last write operation on RTC registers has finished */	    
    if (RTC_GetCounter() == 0x00015180)		/* Reset RTC Counter when Time is 23:59:59 */
    {
      RTC_SetCounter(0x00);		         
      RTC_WaitForLastTask();				/* Wait until last write operation on RTC registers has finished */
    }
  }
}
extern uint8_t NotConfiging;
__mbuf *u1mbuf,*u2mbuf,*u3mbuf,*gmbuf;
void USART1_IRQHandler (void)
{
    __mbuf* buf = u1mbuf;
    uint8_t rtempdata;
    static uint32_t head=0,tail=0;
    static uint16_t lenght=0;
    if(USART_GetFlagStatus(USART1,USART_IT_RXNE)==SET)
	{
		USART_ClearITPendingBit(USART1,USART_IT_RXNE); 
        NotConfiging=0;
		rtempdata = USART_ReceiveData(USART1);
        if(head != 0x01167b28)
        {
            head<<=8;
            head|=rtempdata;
        }
        else
        {
            
            while(buf->pNext!=NULL)
            {
                buf=buf->pNext;
            }
            tail<<=8;
            tail|=rtempdata;
            *(buf->pData+lenght++)=rtempdata;
            if(tail==0x297d7e04 || lenght>252)
            {
                head=rtempdata;
                buf->usable=1;
                buf->datasize=lenght;
                buf->pNext=(__mbuf*)CreateMbuf(252);
                tail=0;
                lenght=0;
            }    
        } 
	}
}
extern __abuf *abuf;
void USART2_IRQHandler (void)
{
    __mbuf* buf = u2mbuf;
    uint8_t rtempdata;
    static uint16_t lenght=0,tail=0;
    #ifdef _DEBUG
        static uint32_t len=0;
    #endif
    if(USART_GetFlagStatus(USART2,USART_IT_RXNE)==SET)
	{
		USART_ClearITPendingBit(USART2,USART_IT_RXNE); 
        rtempdata = USART_ReceiveData(USART2);	
        #ifdef _DEBUG
        *(gmbuf->pData+len++)=rtempdata;
        #endif
        while(buf->pNext!=NULL)
        {
            buf=buf->pNext;
        }
        tail<<=8;
        tail|=rtempdata;
        *(buf->pData+lenght++)=rtempdata;
        buf->datasize=lenght;
        if(tail==0x0d0a || lenght>252)
        //if((lenght>240 && tail==0x0d0a ) || lenght>250)
        {
            *(buf->pData+lenght)=0;
            buf->pNext=(__mbuf*)CreateMbuf(252);
            buf->usable=1;
            tail=0;
            lenght=0;
        }    
	}
}
void USART3_IRQHandler (void)
{
    __mbuf* buf = u3mbuf;
    uint8_t rtempdata;
    static uint32_t head=0,tail=0;
    static uint16_t lenght=0;
    if(USART_GetFlagStatus(USART3,USART_IT_RXNE)==SET)
	{
		USART_ClearITPendingBit(USART3,USART_IT_RXNE); 
		rtempdata = USART_ReceiveData(USART3);	
        if(head != 0x01167b28)
        {
            head<<=8;
            head|=rtempdata;
        }
        else
        {
            while(buf->pNext!=NULL)
            {
                buf=buf->pNext;
            }
            tail<<=8;
            tail|=rtempdata;
            *(buf->pData+lenght++)=rtempdata;
            if(tail==0x297d7e04 || lenght>252)
            {
                head=rtempdata;
                buf->usable=1;
                buf->datasize=lenght;
                buf->pNext=(__mbuf*)CreateMbuf(252);
                tail=0;
                lenght=0;
            }    
        }
    }
}
//uint8_t MH_PowerDown=0;
//void EXTI15_10_IRQHandler(void)
//{
//    //char sendbuf[64]="ZZZZZ#AT+SMSEND=\"";
//    if(EXTI_GetITStatus(EXTI_Line15)==SET)
//    {
//        EXTI_ClearITPendingBit(EXTI_Line15);
//        MH_PowerDown=1;
//    }
//}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
