#ifndef __RTC_H
#define	__RTC_H
#include "stm32f10x.h"

#define FEBRUARY		2
#define	STARTOFTIME		2000  //1970
#define SECDAY			86400L
#define SECYR			(SECDAY * 365)
#define	leapyear(year)		((year) % 4 == 0)
#define	days_in_year(a) 	(leapyear(a) ? 366 : 365)
#define	days_in_month(a) 	(month_days[(a) - 1])

struct rtc_time 
{
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
};


//#define RTCClockOutput_Enable  /* RTC Clock/64 is output on tamper pin(PC.13) */ /* ������������������Ļ�,PC13�ͻ����Ƶ��ΪRTC Clock/64��ʱ�� */   

void RTC_Init(void);
void NVIC_Config(void);
void RTC_Config(void);
void Time_Regulate(struct rtc_time *tm);
void Time_Adjust(void);
void Time_Display(uint32_t TimeVar);
void Time_Display2(uint32_t TimeVar);
void Time_Show(void);
void Time_Show2(void); 
uint8_t USART_Scanf(uint32_t value);

void GregorianDay(struct rtc_time * tm);
void to_tm(uint32_t tim, struct rtc_time * tm);
uint32_t mktimev(struct rtc_time *tm);

u32 TimeCompress(uint8_t *time);

#endif /* __XXX_H */
