#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
#include "mbuf.h"

void USART1_Init(u32 bound);
void USART2_Init(uint32_t band);
void USART3_Init(uint32_t band); //485 RJ45 Sever


void Usart3_SendData(uint8_t *buffer,uint16_t len);
#endif


