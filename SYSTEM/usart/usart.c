#include "sys.h"
#include "usart.h"	  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 0
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*使用microLib的方法*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/
 

  
extern __mbuf *u1mbuf,*u2mbuf,*u3mbuf,*gmbuf;

void USART1_Init(uint32_t band)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	/* config USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	/* USART1 GPIO config */
    /* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
	     
	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure PA8 - USART1 485CTL1 as output  */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	  
	/* USART1 mode config */
	USART_InitStructure.USART_BaudRate = band;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure); 
    USART_Cmd(USART1, ENABLE);
//   USART_ITConfig(USART1,USART_IT_IDLE,ENABLE);
   USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);

//   USART_ClearITPendingBit(USART1,USART_IT_IDLE);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
   NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; //
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   u1mbuf=CreateMbuf(252);
//   gmbuf=CreateMbuf(120);

}
void Usart1_SendData(uint8_t *buffer,uint16_t len)
{  
    uint16_t i;
    if(len==17)
        i=0;
    USART_ClearFlag(USART1,USART_FLAG_TC);	
    for(i=0;i<len;i++)
    {
        USART_SendData(USART1,*buffer++);	 
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET ); 	
    }
}

__mbuf *u2sendbuf;
void USART2_Init(uint32_t band)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	/* config USART1 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	/* USART2 GPIO config */
    /* Configure USART2 Tx (PA.02) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
	     
	/* Configure USART2 Rx (PA.3) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	  
	/* USART2 mode config */
	USART_InitStructure.USART_BaudRate = band;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure); 
    USART_Cmd(USART2, ENABLE);

   USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
    
   NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; //
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
   u2mbuf=CreateMbuf(252);
   memset(u2mbuf->pData,0,252-sizeof(__mbuf));
   u2sendbuf=CreateMbuf(1020);
   #ifdef _DEBUG
   gmbuf=CreateMbuf(1020);
   #endif
}

void Usart2_SendData(uint8_t *buffer,uint16_t len)
{  
    uint16_t i;
    u2sendbuf->datasize=len;
    memcpy(u2sendbuf->pData,buffer,len);
    USART_ClearFlag(USART2,USART_FLAG_TC);	
    for(i=0;i<len;i++)
    {
        USART_SendData(USART2,*buffer++);	 
        while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET ); 	
    }
}
void Usart2_ResendData()
{
    Usart2_SendData(u2sendbuf->pData,u2sendbuf->datasize);
}
void USART3_Init(uint32_t band)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* config USART3 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    /* USART3 GPIO config */
    /* Configure USART3 Tx (PB.10) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure); 
	   
    /* Configure USART3 Rx (PB.11) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* Configure PD.08 - USART3 485CTL1 as output  */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;//|GPIO_Pin_14|GPIO_Pin_15;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);			   
	  
	/* USART3 mode config */
	USART_InitStructure.USART_BaudRate = band;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure); 	
	 
    USART_Cmd(USART3, ENABLE);
    

   USART_ITConfig(USART3,USART_IT_RXNE,ENABLE); 

   NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn; //
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
    
    u3mbuf=CreateMbuf(252);
}
void Usart3_SendData(uint8_t *buffer,uint16_t len)
{  
    uint16_t i;
    if(len==17)
        i=0;
    USART_ClearFlag(USART3,USART_FLAG_TC);	
    for(i=0;i<len;i++)
    {
        USART_SendData(USART3,*buffer++);	 
        while( USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET ); 	
    }
}
