#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "stm32f10x.h"	

/* 获取缓冲区的长度 */
#define TxBufferSize1   (countof(TxBuffer1) - 1)
#define RxBufferSize1   (countof(TxBuffer1) - 1)
#define countof(a)      (sizeof(a) / sizeof(*(a)))
#define BufferSize      (countof(Tx_Buffer)-1)

#define  FLASH_WriteAddress     0x00000
#define  FLASH_ReadAddress      0x00000
#define  FLASH_SectorToErase    FLASH_WriteAddress
#define  sFLASH_ID              0xEF3013
/* Private typedef -----------------------------------------------------------*/
#define SPI_FLASH_PageSize              256
#define SPI_FLASH_PerWritePageSize      256
//#define SPI_FLASH_AllAddress  		524288	           //8*256*256 bytes		     4M bits
#define SPI_FLASH_AllAddress  		    2097152	           //32*256*256 bytes			 16M bits

#define	RECORD_DATA_SIZE   				24		//注：这里设定了需要保存在FLASH中的数据条数，当没有用完16通道时节省了存储空间

/* Private define ------------------------------------------------------------*/
#define W25X_WriteEnable		      0x06 
#define W25X_WriteDisable		      0x04 
#define W25X_ReadStatusReg		      0x05 
#define W25X_WriteStatusReg		      0x01 
#define W25X_ReadData			      0x03 
#define W25X_FastReadData		      0x0B 
#define W25X_FastReadDual		      0x3B 
#define W25X_PageProgram		      0x02 
#define W25X_BlockErase			      0xD8 
#define W25X_SectorErase		      0x20 
#define W25X_ChipErase			      0xC7 
#define W25X_PowerDown			      0xB9 
#define W25X_ReleasePowerDown	      0xAB 
#define W25X_DeviceID			      0xAB 
#define W25X_ManufactDeviceID   	  0x90 
#define W25X_JedecDeviceID		      0x9F 	
#define WIP_Flag                      0x01  /* Write In Progress (WIP) flag */ 
#define Dummy_Byte                    0x00

#define SPI_FLASH_CS_LOW()         GPIO_ResetBits(GPIOG, GPIO_Pin_2)
#define SPI_FLASH_CS_HIGH()        GPIO_SetBits(GPIOG, GPIO_Pin_2)

#define SPI_FLASH_SCK_LOW()        GPIO_ResetBits(GPIOG, GPIO_Pin_4)
#define SPI_FLASH_SCK_HIGH()       GPIO_SetBits(GPIOG, GPIO_Pin_4)

#define SPI_FLASH_MOSI_LOW()       GPIO_ResetBits(GPIOG, GPIO_Pin_5)
#define SPI_FLASH_MOSI_HIGH()      GPIO_SetBits(GPIOG, GPIO_Pin_5) 

#define SPI_FLASH_MISO_IS_LOW      GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_3)==0
#define SPI_FLASH_MISO_IS_HIGH     GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_3)!=0


void FLASH_Init(void);
void SPI_FLASH_SectorErase(u32 SectorAddr);
void SPI_FLASH_BulkErase(void);
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
u32  SPI_FLASH_ReadID(void);
u32  SPI_FLASH_ReadDeviceID(void);
void SPI_FLASH_StartReadSequence(u32 ReadAddr);
void SPI_Flash_PowerDown(void);
void SPI_Flash_WAKEUP(void);

void SPI_FLASH_GPIO_Init(void);
void FLASH_TEST(void);
u8   SPI_FLASH_ReadByte(void);
u8   SPI_FLASH_SendByte(u8 byte);
u8   SPI_FLASH_GetByte(void);
u16  SPI_FLASH_SendHalfWord(u16 HalfWord);
void SPI_FLASH_WriteEnable(void);
void SPI_FLASH_WaitForWriteEnd(void);

u16 FlashWriteAddressOffset(u32 Address);

void DataDownload(u8* pBuffer, u32 ReadAddr, u16 NumPacket);
void DataRecord(void);
 



#endif /* __SPI_FLASH_H */

