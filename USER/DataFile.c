
#include "DataFile.h"
#include "ff.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdio_sdcard.h"
#include "rtc.h"

void SaveData2RecodeFile(_DeviceData *dd)
{
    FATFS *fs;     /* Ponter to the filesystem object */
    FRESULT fres;
    DIR dj={0};         /* Directory search object */
    FILINFO fno={0};    /* File information */
    FIL fp={0};
    char filename[25]={0};
    if(SD_CardIsInserted())
    {
        fs = malloc(sizeof (FATFS));
        fres=f_mount(fs, "0:", 0);
        if(fres==FR_OK)
        {
            memcpy(filename,dd->ID,10);
            strcat(filename,".xls");
            fres=f_findfirst(&dj, &fno, "", filename);
            if(fres == FR_NO_FILE || fres == FR_OK)
            {
                if(fres == FR_NO_FILE || fno.fname[0]==0)
                {
                    f_open(&fp,filename,FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
                    f_printf(&fp,"Data\tTime\tTemp\tHumidity\r\n");
                    strcpy(fno.fname,filename);
                    f_close(&fp);
                }
                fres=f_open(&fp,fno.fname,FA_OPEN_APPEND | FA_WRITE | FA_READ);
                if(fres==FR_OK)
                {
                    f_printf(&fp,"%d-%d-%d\t",dd->time[0]+2000,dd->time[1],dd->time[2]);
                    f_printf(&fp,"%d:%d:%d\t",dd->time[3],dd->time[4],dd->time[5]);
                    f_printf(&fp,"%d.%d\t",(dd->Data1)/10,(dd->Data1)%10);
                    f_printf(&fp,"%d.%d\r\n",(dd->Data2)/10,(dd->Data2)%10);
                    f_close(&fp);
                }
            }
        }
        f_mount(0,"0:",0);
        free(fs);
    }
}

void SaveData2TempFile(_DeviceData *dd)
{
    FATFS *fs;     /* Ponter to the filesystem object */
    uint32_t fres,wbt;
    DIR dj;         /* Directory search object */
    FILINFO fno;    /* File information */
    FIL fp;
    uint32_t t;
    if(SD_CardIsInserted())
    {
        fs = malloc(sizeof (FATFS));
        fres=f_mount(fs, "0:", 0);
        if(fres==FR_OK)
        {
            fres=f_findfirst(&dj, &fno, "", ".Tempdata");
            switch(fres)
            {
                case FR_NO_FILE:
                    f_open(&fp,".Tempdata",FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
                    f_close(&fp);
                    f_chmod(".Tempdata",AM_ARC|AM_HID,AM_ARC|AM_HID); //block for test
                case FR_OK:
                    f_open(&fp,".Tempdata",FA_OPEN_APPEND | FA_WRITE | FA_READ);
                    f_write(&fp,dd->ID,10,&wbt);
                    t=TimeCompress(dd->time);
                    f_write(&fp,&t,4,&wbt);
                    f_write(&fp,&dd->Data1,2,&wbt);
                    f_write(&fp,&dd->Data2,2,&wbt);
                    f_close(&fp);
                    break;
                default:
                    break;
            }
        }
        f_mount(0,"0:",0);
        free(fs);
    }
}





