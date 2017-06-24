#include "stmflash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "delay.h"
//#include "usart.h"
 
 
//��ȡָ����ַ�İ���(16λ����)
//faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
//����ֵ:��Ӧ����.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}
#if STM32_FLASH_WREN	//���ʹ����д   
//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��   
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//��ַ����2.
	}  
} 
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
//pBuffer:����ָ��
//NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽ�
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //������ַ
	u16 secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	u16 secremain; //������ʣ���ַ(16λ�ּ���)	   
 	u16 i;    
	u32 offaddr;   //ȥ��0X08000000��ĵ�ַ
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//�Ƿ���ַ
	FLASH_Unlock();						//����
	offaddr=WriteAddr-STM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	secpos=offaddr/STM_SECTOR_SIZE;			//������ַ  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//����ʣ��ռ��С   
	if(NumToWrite<=secremain)secremain=NumToWrite;//�����ڸ�������Χ
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			FLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//�����������
			for(i=0;i<secremain;i++)//����
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//д����������  
		}else STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(NumToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;				//������ַ��1
			secoff=0;				//ƫ��λ��Ϊ0 	 
		   	pBuffer+=secremain;  	//ָ��ƫ��
			WriteAddr+=secremain;	//д��ַƫ��	   
		   	NumToWrite-=secremain;	//�ֽ�(16λ)���ݼ�
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//��һ����������д����
			else secremain=NumToWrite;//��һ����������д����
		}	 
	};	
	FLASH_Lock();//����
}
#endif

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////WriteAddr:��ʼ��ַ
////WriteData:Ҫд�������
//void Test_Write(u32 WriteAddr,u16 WriteData)   	
//{
//	STMFLASH_Write(WriteAddr,&WriteData,1);//д��һ���� 
//}


void UpDataViaSDCard (FILINFO *fno)
{
    FIL fil;
    FRESULT fr;
    char ch;
    uint8_t tc,d=0,line[24],*tcode,*p,crc;
    
    uint16_t endl,offset=0;
    uint32_t writeaddr,rb;
    int i=0,j=0;
    
    fr=f_open(&fil,fno->fname,FA_READ);
    if(fr!=FR_OK)
        return;
    memset(line,0,24);
    //tcode=(uint8_t*)malloc(512);
    //p=tcode;
    STMFLASH_Read(0x08000000,(uint16_t*)0x20000000,0x200);
    SCB->VTOR = 0x20000000; //�����ж�������
    do
    {
        fr=f_read(&fil,&ch,1,&rb);
        if(fr!=FR_OK || rb==0)
        endl<<=8;
        endl|=ch;
        if(endl==0x0d0a)
        {
            if(line[0]==':')
            {
                crc=0;
                for(j=0;j<(line[1])+4;j++)
                {
                    crc+=line[j+1];
                }
                crc=~crc+1;
                if(crc!=line[line[1]+5])
                {
                    goto nextline;
                }  
                switch(line[4])
                {
                    case 0:
                        offset=line[2]<<8;
                        offset|=line[3];
                        writeaddr=writeaddr&0xffff0000;
                        writeaddr|=offset;
                        STMFLASH_Write(writeaddr,(uint16_t*)(line+5),line[1]/2);
                        break;
                    case 4:
                        writeaddr=line[5]<<24;
                        writeaddr|=line[6]<<16;
                        break;
                    case 1:
                        break;
                    default:
                        break;
                }
            }
            //do write flash
            nextline:
            endl=0;
            d=0;
            i=0;
            memset(line,0,24);
            continue;
        }
        if((ch>='0'&&ch<='9')||(ch>='A'&&ch<='F'))
        {
            if(d==0)
            {
                d=1;
                if(ch>='A')
                {
                    tc=(ch-'A'+10)<<4;
                }
                else
                {
                    tc=(ch-'0')<<4;
                }
            }
            else
            {
                d=0;
                if(ch>='A')
                {
                    tc|=(ch-'A'+10);
                }
                else
                {
                    tc|=(ch-'0');
                }
                line[i++]=tc;
            }
        }
        else
        {
            line[i++]=ch;
        }
    }
    while(ch!=EOF);
    f_close(&fil);
    
    __DSB();                                                          /* Ensure all outstanding memory accesses included
                                                                       buffered write are completed before reset */
    SCB->AIRCR  = (uint32_t)((0x5FAUL << SCB_AIRCR_VECTKEY_Pos)    |
                           (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
                            SCB_AIRCR_SYSRESETREQ_Msk    );         /* Keep priority group unchanged */
    __DSB();                                                          /* Ensure completion of memory access */

    for(;;)                                                           /* wait until reset */
    {
        __NOP();
    }
}













