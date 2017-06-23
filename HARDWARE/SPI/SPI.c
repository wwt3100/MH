/**********************************************************************************
 * 文件名  ：spi_flash.c
 * 描述    ：spi_flash 底层应用函数库   
 * 硬件连接 ----------------------------
 *         | 
 *          ----------------------------
 * 库版本  ：ST3.0.0
 * 广西 张工
**********************************************************************************/
#include "spi.h"	 
#include "SysTick.h"
//#include "stm32flash.h"
//#include "Memory.h"

uint32_t  FlashWriteAddress= 0;
uint32_t  FlashReadAddress= 0;
uint32_t  FlashSaveTime= 10;


__IO uint32_t Device_ID = 0;
__IO uint32_t FlashID = 0;
//__IO TestStatus TransferStatus1 = FAILED;



/*******************************************************************************
* Function Name  : FLASH_Initialize
* Description    : FLASH初始化
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_Init(void)
{
  SPI_FLASH_GPIO_Init();
  if(FlashWriteAddress==0)		 
  {
     if(FlashWriteAddressOffset(0)!=0)	SPI_FLASH_BulkErase();	//
  } 
  else FlashWriteAddress+=FlashWriteAddressOffset(FlashWriteAddress); //
} 

 

//
///*******************************************************************************
//* Function Name  : FLASH_TEST
//* Description    : 测试华邦FLASH
//* Output         : None
//* Return         : None
//*******************************************************************************/
//void DataRecord(void)
//{
//  static u8 i=0;	 
//   if((FlashWriteAddress+RECORD_DATA_SIZE)<SPI_FLASH_AllAddress)
//    {    
//	     RecordData.RTCtime= TimeCompress();		//时间压缩
//		 RecordData.Data1=ChannelData.CH1;//Sensor.CH1*10;
//		 RecordData.Data2=ChannelData.CH2;//Sensor.CH2*10;
//		 RecordData.Data3=ChannelData.CH3;//Sensor.CH3*10;
//		 RecordData.Data4=ChannelData.CH4;//Sensor.CH4*10;
//		 RecordData.Data5=ChannelData.CH5;//Sensor.CH5*10;
//		 RecordData.Data6=ChannelData.CH6;//Sensor.CH6*10;
//		 RecordData.Data7=ChannelData.CH7;//Sensor.CH7*10;
//		 RecordData.Data8=ChannelData.CH8;//Sensor.CH8*10;
//		 RecordData.Data9=ChannelData.CH9;//Sensor.CH9*10;
//		 RecordData.Data10=ChannelData.CH10;//Sensor.CH10*10;
//		 //RecordData.Data11=ChannelData.CH11;//Sensor.CH11*10;
//		 //RecordData.Data12=ChannelData.CH12;//Sensor.CH12*10;
//		 //RecordData.Data13=ChannelData.CH13;//Sensor.CH13*10;
//		 //RecordData.Data14=ChannelData.CH14;//Sensor.CH14*10;
//		 //RecordData.Data15=ChannelData.CH15;//Sensor.CH15*10;
//		 //RecordData.Data16=ChannelData.CH16;//Sensor.CH16*10;
//		 
//		 if(FlashWriteAddress%RECORD_DATA_SIZE!=0)
//		 {
//		   FlashWriteAddress=(FlashWriteAddress/RECORD_DATA_SIZE)*RECORD_DATA_SIZE+RECORD_DATA_SIZE;//
//		 } 
//	    
//	     SPI_FLASH_BufferWrite((u8 *)&RecordData, FlashWriteAddress,RECORD_DATA_SIZE);		         
//		 FlashWriteAddress=FlashWriteAddress+RECORD_DATA_SIZE;	 			  
//		 Para[0]=FlashWriteAddress>>24;
//		 Para[1]=FlashWriteAddress>>16;
//		 Para[2]=FlashWriteAddress>>8;
//		 Para[3]=FlashWriteAddress>>0; 
//		 //STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)Para,sizeof(Para));
//		 if(i==0)STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)Para,sizeof(Para));	 
//		 i++;
//		 if(i>=50)i=0;  	    
//	}
//	
//}

/*******************************************************************************
* Function Name  : FLASH_TEST
* Description    : 测试华邦FLASH
* Output         : None
* Return         : None
*******************************************************************************/
//void DataDownload(u8* pBuffer, u32 ReadAddr, u16 NumPacket)
//{	 
//     if(ReadAddr%RECORD_DATA_SIZE!=0)							 //
//	 {
//		   ReadAddr=(ReadAddr/RECORD_DATA_SIZE)*RECORD_DATA_SIZE;//
//	 } 
//	 
//	 while(NumPacket--)
//	 {
//	    SPI_FLASH_BufferRead(pBuffer,ReadAddr,RECORD_DATA_SIZE);		            
//		MemoryReset(pBuffer+RECORD_DATA_SIZE,36-RECORD_DATA_SIZE);					
//	    pBuffer=pBuffer+36;															
//	    ReadAddr=ReadAddr+RECORD_DATA_SIZE;											
//	 }
//	 //SPI_FLASH_BufferRead(pBuffer,ReadAddr,(RECORD_DATA_SIZE*NumPacket));		    
//}
/*******************************************************************************
* Function Name  :FlashWriteAddressOffset
* Description    : 
* Output         : None
* Return         : None
*******************************************************************************/
u16 FlashWriteAddressOffset(u32 Address)
{
  u8 Buffer[RECORD_DATA_SIZE];
  u8 timeout=0;
  u16 OffsetAddress=0;
  while(timeout<100)
  {
  	SPI_FLASH_BufferRead(Buffer,Address,RECORD_DATA_SIZE);
	if(Buffer[0]==0xff&&Buffer[1]==0xff&&Buffer[2]==0xff&&Buffer[3]==0xff)break;
	else
	{
	    OffsetAddress+=RECORD_DATA_SIZE;
	    Address+=RECORD_DATA_SIZE;	
	} 
	timeout++;
  }	 
 
 return  OffsetAddress;
}


/*******************************************************************************
* Function Name  : SPI_FLASH_Init
* Description    : Initializes the peripherals used by the SPI FLASH driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Enable SPI1 and GPIO clocks */
  /*!< SPI_FLASH_SPI_CS_GPIO, SPI_FLASH_SPI_MOSI_GPIO, 
       SPI_FLASH_SPI_MISO_GPIO, SPI_FLASH_SPI_DETECT_GPIO 
       and SPI_FLASH_SPI_SCK_GPIO Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG , ENABLE);   
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				/*!< Configure SPI_FLASH_SPI pins: SCK */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOG, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;			   /*!< Configure SPI_FLASH_SPI_CS_PIN pin: SPI_FLASH Card CS pin */
  GPIO_Init(GPIOG, &GPIO_InitStructure);
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;			   /*!< Configure SPI_FLASH_SPI pins: MOSI */
  GPIO_Init(GPIOG, &GPIO_InitStructure);	   
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;			  /*!< Configure SPI_FLASH_SPI pins: MISO */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOG, &GPIO_InitStructure);  	   
  
  SPI_FLASH_CS_HIGH(); 								  /* Deselect the FLASH: Chip Select high */
   
}  

/*******************************************************************************
* Function Name  : SPI_FLASH_SectorErase
* Description    : Erases the specified FLASH sector.	摩除一个扇区 16*256 bytes
* Input          : SectorAddr: address of the sector to erase.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_SectorErase(u32 SectorAddr)
{   
  SPI_FLASH_WriteEnable();			/* Send write enable instruction */
  SPI_FLASH_WaitForWriteEnd();
  		
  /* Sector Erase 16*256 bytes */  	   
  SPI_FLASH_CS_LOW();			                        /* Select the FLASH: Chip Select low */	  
  SPI_FLASH_SendByte(W25X_SectorErase);	                /* Send Sector Erase instruction */  
  SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);    /* Send SectorAddr high nibble address byte */	  
  SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);		/* Send SectorAddr medium nibble address byte */  
  SPI_FLASH_SendByte(SectorAddr & 0xFF);				/* Send SectorAddr low nibble address byte */  
  SPI_FLASH_CS_HIGH();				                    /* Deselect the FLASH: Chip Select high */ 
    
  SPI_FLASH_WaitForWriteEnd();		                    /* Wait the end of Flash writing */
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BulkErase
* Description    : Erases the entire FLASH.	摩除整片	 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BulkErase(void)
{   
  SPI_FLASH_WriteEnable();				   /* Send write enable instruction */ 
  /* Bulk Erase   */   
  SPI_FLASH_CS_LOW();					  /* Select the FLASH: Chip Select low */   
  SPI_FLASH_SendByte(W25X_ChipErase);	  /* Send Bulk Erase instruction  */  
  SPI_FLASH_CS_HIGH();					  /* Deselect the FLASH: Chip Select high */  
   
  SPI_FLASH_WaitForWriteEnd();			  /* Wait the end of Flash writing */
}

/*******************************************************************************
* Function Name  : SPI_FLASH_PageWrite
* Description    : Writes more than one byte to the FLASH with a single WRITE
*                  cycle(Page WRITE sequence). The number of byte can't exceed
*                  the FLASH page size.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH,
*                    must be equal or less than "SPI_FLASH_PageSize" value.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
 
  SPI_FLASH_WriteEnable();					        /* Enable the write access to the FLASH */	  
  SPI_FLASH_CS_LOW();						    	/* Select the FLASH: Chip Select low */	   
  SPI_FLASH_SendByte(W25X_PageProgram);			   /* Send "Write to Memory " instruction */    
  SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);/* Send WriteAddr high nibble address byte to write to */   
  SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);   /* Send WriteAddr medium nibble address byte to write to */   
  SPI_FLASH_SendByte(WriteAddr & 0xFF);			  /* Send WriteAddr low nibble address byte to write to */ 
  if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
  {
     NumByteToWrite = SPI_FLASH_PerWritePageSize;
     //printf("\n\r Err: SPI_FLASH_PageWrite too large!");
  } 
  while (NumByteToWrite--)					      /* while there is data to be written on the FLASH */
  {	    
    SPI_FLASH_SendByte(*pBuffer);			      /* Send the current byte */     
    pBuffer++;							      	 /* Point on the next byte to be written */
  }  
  SPI_FLASH_CS_HIGH();					         /* Deselect the FLASH: Chip Select high */   
  SPI_FLASH_WaitForWriteEnd();			         /* Wait the end of Flash writing */
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BufferWrite
* Description    : Writes block of data to the FLASH. In this function, the
*                  number of WRITE cycles are reduced, using Page WRITE sequence.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % SPI_FLASH_PageSize;		 //	SPI_FLASH_PageSize=256
  count = SPI_FLASH_PageSize - Addr;
  NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;		//需要多少页
  NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;	//需要多少单个

  if (Addr == 0)                                  /* WriteAddr is SPI_FLASH_PageSize aligned  */
  {
    if (NumOfPage == 0)                           /* NumByteToWrite < SPI_FLASH_PageSize */
    {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
    }
    else                                         /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }

      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else                                           /* WriteAddr is not SPI_FLASH_PageSize aligned  */
  {
    if (NumOfPage == 0)                          /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      if (NumOfSingle > count)                   /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
      {
        temp = NumOfSingle - count;

        SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
      }
      else
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else                                        /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
      NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

      SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }

      if (NumOfSingle != 0)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BufferRead
* Description    : Reads a block of data from the FLASH.
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the FLASH.
*                  - ReadAddr : FLASH's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
 
  SPI_FLASH_CS_LOW();		                       /* Select the FLASH: Chip Select low */	  
  SPI_FLASH_SendByte(W25X_ReadData);               /* Send "Read from Memory " instruction */    
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16); /* Send ReadAddr high nibble address byte to read from */ 
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);	   /* Send ReadAddr medium nibble address byte to read from */	 
  SPI_FLASH_SendByte(ReadAddr & 0xFF);			   /* Send ReadAddr low nibble address byte to read from */

  while (NumByteToRead--)                          /* while there is data to be read */
  {	    
    *pBuffer = SPI_FLASH_SendByte(Dummy_Byte);	   /* Read a byte from the FLASH */    
     pBuffer++;									   /* Point to the next location where the byte read will be saved */
  }	  
  SPI_FLASH_CS_HIGH();							  /* Deselect the FLASH: Chip Select high */
}

/*******************************************************************************
* Function Name  : SPI_FLASH_ReadID
* Description    : Reads FLASH identification.
* Input          : None
* Output         : None
* Return         : FLASH identification
*******************************************************************************/
u32 SPI_FLASH_ReadID(void)
{
  u32 Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;  
  SPI_FLASH_CS_LOW();					       /* Select the FLASH: Chip Select low */	 
  SPI_FLASH_SendByte(W25X_JedecDeviceID);      /* Send "RDID " instruction */	  
  Temp0 = SPI_FLASH_SendByte(Dummy_Byte);      /* Read a byte from the FLASH */	  
  Temp1 = SPI_FLASH_SendByte(Dummy_Byte);      /* Read a byte from the FLASH */	  
  Temp2 = SPI_FLASH_SendByte(Dummy_Byte);      /* Read a byte from the FLASH */	  
  SPI_FLASH_CS_HIGH();					       /* Deselect the FLASH: Chip Select high */  
  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;	
  return Temp;
}
/*******************************************************************************
* Function Name  : SPI_FLASH_ReadID
* Description    : Reads FLASH identification.
* Input          : None
* Output         : None
* Return         : FLASH identification
*******************************************************************************/
u32 SPI_FLASH_ReadDeviceID(void)
{
  u8 Temp = 0;   
  SPI_FLASH_CS_LOW();				      /* Select the FLASH: Chip Select low */   
  SPI_FLASH_SendByte(W25X_DeviceID);      /* Send "RDID " instruction */
  SPI_FLASH_SendByte(0);
  SPI_FLASH_SendByte(0);
  SPI_FLASH_SendByte(0);    
  Temp = SPI_FLASH_SendByte(0);          /* Read a byte from the FLASH */    
  SPI_FLASH_CS_HIGH();			         /* Deselect the FLASH: Chip Select high */	
  return Temp;
}
/*******************************************************************************
* Function Name  : SPI_FLASH_StartReadSequence
* Description    : Initiates a read data byte (READ) sequence from the Flash.
*                  This is done by driving the /CS line low to select the device,
*                  then the READ instruction is transmitted followed by 3 bytes
*                  address. This function exit and keep the /CS line low, so the
*                  Flash still being selected. With this technique the whole
*                  content of the Flash is read with a single READ instruction.
* Input          : - ReadAddr : FLASH's internal address to read from.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_StartReadSequence(u32 ReadAddr)
{
  
  SPI_FLASH_CS_LOW();					    	    /* Select the FLASH: Chip Select low */	 
  SPI_FLASH_SendByte(W25X_ReadData);		        /* Send "Read from Memory " instruction */

  /* Send the 24-bit address of the address to read from -----------------------*/      
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);  /* Send ReadAddr high nibble address byte */	 
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);	    /* Send ReadAddr medium nibble address byte */  
  SPI_FLASH_SendByte(ReadAddr & 0xFF);				/* Send ReadAddr low nibble address byte */
}

/*******************************************************************************
* Function Name  : SPI_FLASH_ReadByte
* Description    : Reads a byte from the SPI Flash.
*                  This function must be used only if the Start_Read_Sequence
*                  function has been previously called.
* Input          : None
* Output         : None
* Return         : Byte Read from the SPI Flash.
*******************************************************************************/
u8 SPI_FLASH_ReadByte(void)
{
  return (SPI_FLASH_SendByte(Dummy_Byte));
}

/*******************************************************************************
* Function Name  : SPI_FLASH_SendByte
* Description    : Sends a byte through the SPI interface and return the byte received from the SPI bus.                  
* Input          : aByte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
u8 SPI_FLASH_SendByte(u8 aByte)
{
   u8 i,value;
    SPI_FLASH_SCK_LOW();
    value = 0;
    for(i=0;i<8;i++)    // 写寄存器
    {
        value <<= 1;

        if((aByte & 0x80)==0x80)    //SPI把aByte发送到从机
            SPI_FLASH_MOSI_HIGH();
        else
            SPI_FLASH_MOSI_LOW();
        aByte <<= 1; 
        SPI_FLASH_SCK_HIGH();
        if(SPI_FLASH_MISO_IS_HIGH)       //SPI从机读回状态字
            value |= 0x01;	
        SPI_FLASH_SCK_LOW();
    }	
    return value;
} 


///*******************************************************************************
//* Function Name  : SPI_FLASH_SendHalfWord
//* Description    : Sends a Half Word through the SPI interface and return the
//*                  Half Word received from the SPI bus.
//* Input          : Half Word : Half Word to send.
//* Output         : None
//* Return         : The value of the received Half Word.
//*******************************************************************************/
//u16 SPI_FLASH_SendHalfWord(u16 HalfWord)
//{
//  /* Loop while DR register in not emplty */
//  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
//
//  /* Send Half Word through the SPI1 peripheral */
//  SPI_I2S_SendData(SPI1, HalfWord);
//
//  /* Wait to receive a Half Word */
//  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
//
//  /* Return the Half Word read from the SPI bus */
//  return SPI_I2S_ReceiveData(SPI1);
//}

/*******************************************************************************
* Function Name  : SPI_FLASH_WriteEnable
* Description    : Enables the write access to the FLASH.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_WriteEnable(void)
{
 
  SPI_FLASH_CS_LOW();					  /* Select the FLASH: Chip Select low */   
  SPI_FLASH_SendByte(W25X_WriteEnable);	  /* Send "Write Enable" instruction */	  
  SPI_FLASH_CS_HIGH();					  /* Deselect the FLASH: Chip Select high */
}

/*******************************************************************************
* Function Name  : SPI_FLASH_WaitForWriteEnd
* Description    : Polls the status of the Write In Progress (WIP) flag in the
*                  FLASH's status  register  and  loop  until write  opertaion has completed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_WaitForWriteEnd(void)
{
  u8 FLASH_Status = 0;	   
  SPI_FLASH_CS_LOW();							   /* Select the FLASH: Chip Select low */	  
  SPI_FLASH_SendByte(W25X_ReadStatusReg);		   /* Send "Read Status Register" instruction */    
  do                                               /* Loop as long as the memory is busy with a write cycle */
  {	    
    FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte); /* Send a dummy byte to generate the clock needed by the FLASH*/ 	
  }												   /*and put the value of the status register in FLASH_Status variable */
  while ((FLASH_Status & WIP_Flag) == SET);        /* Write in progress */	   
  SPI_FLASH_CS_HIGH();						       /* Deselect the FLASH: Chip Select high */
}

/*******************************************************************************
* Function Name  : SPI_Flash_PowerDown
* Description    : Flash PowerDown 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_Flash_PowerDown(void)   
{    
  SPI_FLASH_CS_LOW();							  /* Select the FLASH: Chip Select low */  
  SPI_FLASH_SendByte(W25X_PowerDown);			  /* Send "Power Down" instruction */    
  SPI_FLASH_CS_HIGH();							  /* Deselect the FLASH: Chip Select high */
}
   
/*******************************************************************************
* Function Name  : SPI_Flash_WAKEUP
* Description    : PowerDown
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_Flash_WAKEUP(void)   
{  
  SPI_FLASH_CS_LOW();   					   /* Select the FLASH: Chip Select low */
  SPI_FLASH_SendByte(W25X_ReleasePowerDown);   /* Send "Power Down" instruction */
  SPI_FLASH_CS_HIGH();                         /* Deselect the FLASH: Chip Select high */ //wait for TRES1
}

 


