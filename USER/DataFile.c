
#include "DataFile.h"
#include "led.h" 

//static DWORD SaveNumSize=0;
extern volatile _HostStat hstat;
extern FATFS *fs;

void SaveData2RecodeFile(_DeviceData *dd)
{
//    FATFS *fs;     /* Ponter to the filesystem object */
    FRESULT fres=FR_INVALID_DRIVE;
    DIR dj={0};         /* Directory search object */
    FILINFO *fno;    /* File information */
    FIL *fp;
    char filename[25]={0};
    if(SD_CardIsInserted())
    {
//        fs = malloc(1020);//malloc(sizeof (FATFS));
        fp = malloc(636);
        fno= malloc(380);
        fres=f_mount(fs, "0:", 0);
        if(fres==FR_OK)
        {
            memcpy(filename,dd->ID,10);
            strcat(filename,".xls");
            fres=f_findfirst(&dj, fno, "", filename);
            if(fres == FR_NO_FILE || fres == FR_OK)
            {
//                if(fres == FR_NO_FILE || fno.fname[0]==0)   //定义文件头
//                {
//                    f_open(&fp,filename,FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
//                    f_printf(&fp,"Data\tTime\tTemp\tHumidity\r\n");
//                    strcpy(fno.fname,filename);
//                    f_close(&fp);
//                }
                fres=f_open(fp,filename,FA_OPEN_APPEND | FA_WRITE | FA_READ);   //打开文件,如果不存在则新建
                if(fres==FR_OK)
                {
                    f_printf(fp,"%d-%d-%d\t",dd->time[0]+2000,dd->time[1],dd->time[2]);
                    f_printf(fp,"%d:%d:%d\t",dd->time[3],dd->time[4],dd->time[5]);
                    f_printf(fp,"%d.%d\t",(dd->Data1)/10,(dd->Data1)%10);
                    f_printf(fp,"%d.%d\r\n",(dd->Data2)/10,(dd->Data2)%10);
                    f_close(fp);
                }
            }
        }
        f_mount(0,"0:",1);
//        free(fs);
        free(fp);
        free(fno);
    }
    hstat.SDCardStat=fres;
}
// old version
//void SaveData2TempFile(_DeviceData *dd)
//{
////    FATFS *fs;     /* Ponter to the filesystem object */
//    uint32_t fres=FR_INVALID_DRIVE,wbt;
//    DIR dj;         /* Directory search object */
//    FILINFO *fno;    /* File information */
//    FIL *fp;
//    uint32_t t;
//    if(SD_CardIsInserted())
//    {
////        fs = malloc(1020);//malloc(sizeof (FATFS));
//        fp = malloc(636);
//        fno= malloc(380);
//        fres=f_mount(fs, "0:", 0);
//        if(fres==FR_OK)
//        {
//            fres=f_findfirst(&dj, fno, "", ".Tempdata");
//            switch(fres)
//            {
//                case FR_NO_FILE:
//                    //f_open(&fp,".Tempdata",FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
//                    //f_close(&fp);
//                    //f_chmod(".Tempdata",AM_ARC|AM_HID,AM_ARC|AM_HID); //block for test
//                case FR_OK:
//                    f_open(fp,".Tempdata",FA_OPEN_APPEND | FA_WRITE | FA_READ);
////                    if(SaveNumSize==0)
////                    {
////                        SaveNumSize=f_size(fp);
////                        //SaveNum/=18;
////                    }
////                    f_lseek(fp,SaveNumSize);
//                    f_write(fp,dd->ID,10,&wbt);
//                    t=TimeCompress(dd->time);
//                    f_write(fp,&t,4,&wbt);
//                    f_write(fp,&dd->Data1,2,&wbt);
//                    f_write(fp,&dd->Data2,2,&wbt);
//                    f_close(fp);
//                    
//                    SaveNumSize+=18;
//                    break;
//                default:
//                    break;
//            }
//        }
////        f_mount(0,"0:",1);
////        free(fs);
//        free(fp);
//        free(fno);
//    }
//    hstat.SDCardStat=fres;
//}
static FIL sfp;
void SaveData2TempFile(_DeviceData *dd)
{
    uint32_t fres=FR_INVALID_DRIVE,wbt;
    //DIR dj;         /* Directory search object */
    //FILINFO *fno;    /* File information */
    uint32_t t;
    static uint8_t SDStat;
    for(;;)
    {
        switch(SDStat)
        {
            case eSDStat_NotOpen:
                //fno= malloc(380);
                fres=f_mount(fs, "0:", 0);
                if(fres==FR_OK)
                {
                      fres=f_open(&sfp,".Tempdata",FA_OPEN_APPEND | FA_WRITE | FA_READ);
                      if(fres==FR_OK)
                      {
                          SDStat++;
                      }
                      else
                      {
                          hstat.SDCardStat=fres;
                          return;
                      }
                }
                else
                {
                    hstat.SDCardStat=fres;
                    return;
                }
                //free(fno);
                break;
            case eSDStat_Write:
                LED1(Bit_RESET);
                fres=f_write(&sfp,dd->ID,10,&wbt);
                t=TimeCompress(dd->time);
                fres|=f_write(&sfp,&t,4,&wbt);
                fres|=f_write(&sfp,&dd->Data1,2,&wbt);
                fres|=f_write(&sfp,&dd->Data2,2,&wbt);
                fres|=f_sync(&sfp);
                LED1(Bit_RESET);
                hstat.SDCardStat=fres;
                if(fres!=FR_OK)
                {
                    SDStat--;
                    break;
                }
                return;
            default:
                break;
        }
    }
}
FRESULT ReadTempFileSize(FSIZE_t *size)
{
    FRESULT fres=FR_INVALID_DRIVE;
    FILINFO *fno;
    fno = malloc(380);
    fres=f_stat(".Tempdata",fno);
    *size=fno->fsize;
    free(fno);
    return fres;
}
// Old Version
//FRESULT ReadTempFileSize(FSIZE_t *size)
//{
////    FATFS *fs;     /* Ponter to the filesystem object */
//    FRESULT fres=FR_INVALID_DRIVE;
//    FIL *fp;
//    *size=0;
//    if(SD_CardIsInserted())
//    {
////        fs = malloc(636);//malloc(sizeof (FATFS));
//        fp = malloc(636);
//        fres=f_mount(fs, "0:", 0);
//        if(fres==FR_OK)
//        {
//            fres=f_open(fp,".Tempdata",FA_OPEN_EXISTING);
//            switch(fres)
//            {
//                case FR_OK:
//                    *size=f_size(fp);
//                default:
//                    break;
//            }
//            f_close(fp);
////            f_mount(0,"0:",0);
//        }
////        free(fs);
//        free(fp);
//    }
//    hstat.SDCardStat=fres;
//    return fres;
//}

//void EXTI9_5_IRQHandler(void)
//{
//    if(EXTI_GetITStatus(EXTI_Line7)==SET)
//    {
//        EXTI_ClearITPendingBit(EXTI_Line7);
//        f_sync(&sfp);
//    }
//}

