

#include "SMSAlarm.h"



uint8_t *SMSAlarmMessage=0;




uint8_t SMSAlarm()
{
    char *Head={"ZZZZ#AT+SMSEND="};
    char *Sendbuf;
    if(SMSAlarmMessage==NULL)
    {
        return 1;
    }
    
    
    
    
    free(SMSAlarmMessage);
    SMSAlarmMessage=NULL;
    return 0;
}











