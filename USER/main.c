#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	  
#include "usmart.h" 
#include "sdio_sdcard.h"  
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"    



 int main(void)
 {	 
 	u32 total,free;
	u8 t=0;	
	u8 res=0;	    	    

	delay_init();	    	 //延时函数初始化	  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
	usmart_dev.init(72);		//初始化USMART		
 	LED_Init();		  			//初始化与LED连接的硬件接口

	W25QXX_Init();				//初始化W25Q128 
	while(SD_Init())//检测不到SD卡
	{
	}
 	exfuns_init();							//为fatfs相关变量申请内存				 
  	f_mount(fs[0],"0:",1); 					//挂载SD卡 
 	res=f_mount(fs[1],"1:",1); 				//挂载FLASH.	
	if(res==0X0D)//FLASH磁盘,FAT文件系统错误,重新格式化FLASH
	{
		res=f_mkfs("1:",1,4096);//格式化FLASH,1,盘符;1,不需要引导区,8个扇区为1个簇
		if(res==0)
		{
			f_setlabel((const TCHAR *)"1:HUATO");	//设置Flash磁盘的名字为：ALIENTEK
			
		}
        else
            ;	//格式化失败
		delay_ms(1000);
	}													    
	while(exf_getfree("0",&total,&free))	//得到SD卡的总容量和剩余容量
	{
		
		delay_ms(200);
		delay_ms(200);
	}													  			    
	while(1)
	{
		t++; 
		delay_ms(200);		 			   
	} 
}
















