#include "rtc.h"
#include "stdio.h"

struct rtc_time systmtime;

static int month_days[12] = {	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
u8 const *WEEK_STR[] = {"��", "һ", "��", "��", "��", "��", "��"};


void RTC_Init(void)
{

  NVIC_Config();  	/* ����RTC���ж����ȼ� */
  //if(1)//
  if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
  { 
    RTC_Config();	  /* RTC Configuration */     
    //Time_Adjust();	  /* Adjust time by values entred by the user on the hyperterminal */
    BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
  }
  else
  {	   
    RTC_WaitForSynchro(); 	          /* Wait for RTC registers synchronization */      
    RTC_ITConfig(RTC_IT_SEC, ENABLE); /* Enable the RTC Second */     
    RTC_WaitForLastTask();	          /* Wait until last write operation on RTC registers has finished */
  }

#ifdef RTCClockOutput_Enable
 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	   /* Enable PWR and BKP clocks */	  
    PWR_BackupAccessCmd(ENABLE);		  /* Allow access to BKP Domain */   
    BKP_TamperPinCmd(DISABLE); /* Disable the Tamper Pin .To output RTCCLK/64 on Tamper pin, the tamper functionality must be disabled */	  
    BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);	/* Enable RTC Clock Output on Tamper Pin */
#endif	 
  //RCC_ClearFlag();	    /* Clear reset flags */  
  //Time_Show();			/* Display time in infinite loop */
  to_tm(RTC_GetCounter(), &systmtime);       /*�����������ʱ��*/  
}

/*******************************************************************************************
 * ��������RTC_Configuration
 * ����  ������RTC
 * ����  ����
 * ���  ����
 *******************************************************************************************/
void RTC_Config(void)
{
 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	 /* Enable PWR and BKP clocks */ 																			  
  PWR_BackupAccessCmd(ENABLE);												 /* Allow access to BKP Domain */   																			 
  BKP_DeInit();																 /* Reset Backup Domain */	   
  RCC_LSEConfig(RCC_LSE_ON);												 /* Enable LSE */	  
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);						 /* Wait till LSE is ready */   
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);									 /* Select LSE as RTC Clock Source */   
  RCC_RTCCLKCmd(ENABLE);													 /* Enable RTC Clock */	  
  RTC_WaitForSynchro();														 /* Wait for RTC registers synchronization */   
  RTC_WaitForLastTask();													 /* Wait until last write operation on RTC registers has finished */   
  RTC_ITConfig(RTC_IT_SEC, ENABLE);											 /* Enable the RTC Second */    
  RTC_WaitForLastTask();													 /* Wait until last write operation on RTC registers has finished */ 
  RTC_SetPrescaler(32767);                                                   /* Set RTC prescaler: set RTC period to 1sec . RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */   
  RTC_WaitForLastTask();													 /* Wait until last write operation on RTC registers has finished */
}

void NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  /* Enable the RTC Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * WARNING: this function will overflow on 2106-02-07 06:28:16 on
 * machines were long is 32-bit! (However, as time_t is signed, we
 * will already get problems at other places on 2038-01-19 03:14:08)
 */
u32 mktimev(struct rtc_time tm)
{
	if (0 >= (int) (tm.tm_mon -= 2)) 
	{	/* 1..12 -> 11,12,1..10 */
		tm.tm_mon += 12;		/* Puts Feb last since it has leap day */
		tm.tm_year -= 1;
	}

	return 
	((((u32) (tm.tm_year/4 - tm.tm_year/100 + tm.tm_year/400 + 367*tm.tm_mon/12 + tm.tm_mday) + tm.tm_year*365 - 730456  //719499	
	    )*24 + tm.tm_hour /* now have hours */
	  )*60 + tm.tm_min /* now have minutes */
	)*60 + tm.tm_sec; /* finally seconds */
}
//�ж��Ƿ������꺯��
//�·�   1  2  3  4  5  6  7  8  9  10 11 12
//����   31 29 31 30 31 30 31 31 30 31 30 31
//������ 31 28 31 30 31 30 31 31 30 31 30 31
//����:���
//���:������ǲ�������.1,��.0,����
u8 Is_Leap_Year(u16 year)
{			  
	if(year%4==0) //�����ܱ�4����
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//�����00��β,��Ҫ�ܱ�400���� 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	 			   
//����ʱ��
//�������ʱ��ת��Ϊ����
//��1970��1��1��Ϊ��׼
//1970~2099��Ϊ�Ϸ����
//����ֵ:0,�ɹ�;����:�������.
//�·����ݱ�											 
u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //���������ݱ�	  
//ƽ����·����ڱ�
const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
u8 RTC_Set(struct rtc_time *tm)
{
	u16 t;
	u32 seccount=0;
	if(tm->tm_year<1970||tm->tm_year>2099)return 1;	   
	for(t=1970;t<tm->tm_year;t++)	//��������ݵ��������
	{
		if(Is_Leap_Year(t))seccount+=31622400;//�����������
		else seccount+=31536000;			  //ƽ���������
	}
	tm->tm_mon-=1;
	for(t=0;t<tm->tm_mon;t++)	   //��ǰ���·ݵ����������
	{
		seccount+=(u32)mon_table[t]*86400;//�·����������
		if(Is_Leap_Year(tm->tm_year)&&t==1)seccount+=86400;//����2�·�����һ���������	   
	}
	seccount+=(u32)(tm->tm_mday)*86400;//seccount+=(u32)(tm->tm_mday-1)*86400;//��ǰ�����ڵ���������� 
	seccount+=(u32)tm->tm_hour*3600;//Сʱ������
    seccount+=(u32)tm->tm_min*60;	 //����������
	seccount+=tm->tm_sec;//�������Ӽ���ȥ

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��  
	PWR_BackupAccessCmd(ENABLE);	//ʹ��RTC�ͺ󱸼Ĵ������� 
	RTC_SetCounter(seccount);	//����RTC��������ֵ
	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������  	
    BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    
	return 0;	    
}
 /*******************************************************************************
 * ��������to_tm
 * ����  ������RTCʱ��
 * ����  ��tim  RTC���㵽��������tm ʵ�ʵ�������ʱ����
 * ���  ����
 *******************************************************************************/
void to_tm(u32 tim, struct rtc_time * tm)
{
	register u32    i;
	register long   hms, day;

	day = tim / SECDAY;
	hms = tim % SECDAY;

	/* Hours, minutes, seconds are easy */
	tm->tm_hour = hms / 3600;
	tm->tm_min = (hms % 3600) / 60;
	tm->tm_sec = (hms % 3600) % 60;

	/* Number of years in days */ /*�����ǰ��ݣ���ʼ�ļ������Ϊ1970��*/
	for (i = STARTOFTIME; day >= days_in_year(i); i++) {
		day -= days_in_year(i);
	}
	tm->tm_year = i;

	/* Number of months in days left */ /*���㵱ǰ���·�*/
	if (leapyear(tm->tm_year)) {
		days_in_month(FEBRUARY) = 29;
	}
	for (i = 1; day >= days_in_month(i); i++) {
		day -= days_in_month(i);
	}
	days_in_month(FEBRUARY) = 28;
	tm->tm_mon = i;

	/* Days are what is left over (+1) from all that. *//*���㵱ǰ����*/
	tm->tm_mday = day + 1;

	/*
	 * Determine the day of week
	 */
	GregorianDay(tm);
	
}

 /*******************************************************************************
 * ��������GregorianDay
 * ����  �����㹫��	 This only works for the Gregorian calendar - i.e. after 1752 (in the UK)to_tm(RTC_GetCounter(), &systmtime);
 * ����  ����
 * ���  ����
 *******************************************************************************/
void GregorianDay(struct rtc_time * tm)
{
	int leapsToDate;
	int lastYear;
	int day;
	int MonthOffset[] = { 0,31,59,90,120,151,181,212,243,273,304,334 };

	lastYear=tm->tm_year-1;	
	leapsToDate = lastYear/4 - lastYear/100 + lastYear/400;      	/*����ӹ�ԪԪ�굽������ǰһ��֮��һ�������˶��ٸ�����*/      
	if((tm->tm_year%4==0) &&((tm->tm_year%100!=0) || (tm->tm_year%400==0)) &&(tm->tm_mon>2)) 	 /*������������һ��Ϊ���꣬�Ҽ������·���2��֮����������1�����򲻼�1*/
	{  		
		day=1;	  /* We are past Feb. 29 in a leap year*/
	} 
	else 
	{
		day=0;
	}
	day += lastYear*365 + leapsToDate + MonthOffset[tm->tm_mon-1] + tm->tm_mday; /*����ӹ�ԪԪ��Ԫ������������һ���ж�����*/
	tm->tm_wday=day%7;
}
u32 TimeCompress(uint8_t *time)
{							//   ��      ��  	 ��		 ʱ		  ��	   ��  
   u32 t=0;				//[1111,11][11,11][11,111][1,1111,][1111,11][11,1111]
   if(time==NULL)
   {
       t|=(systmtime.tm_year%100)<<26;//99��	6bits
       t|=systmtime.tm_mon<<22;		//12��  4bits
       t|=systmtime.tm_mday<<17;		//31��  5bits
       t|=systmtime.tm_hour<<12;	    //24ʱ  5bits
       t|=systmtime.tm_min<<6;		//59��  6bits
       t|=systmtime.tm_sec;			//59��  6bits
   }
   else
   {
       t|=(time[0]%100)<<26;//99��	6bits
       t|=time[1]<<22;		//12��  4bits
       t|=time[2]<<17;		//31��  5bits
       t|=time[3]<<12;	    //24ʱ  5bits
       t|=time[4]<<6;		//59��  6bits
       t|=time[5];			//59��  6bits
   }
   return t ;
}
