#ifndef __MBUF_H
#define __MBUF_H

#include "stm32f10x.h"
#pragma pack(2)
typedef struct TagMbuf	
{
    struct TagMbuf	*pNext;
    uint8_t packed;
    uint8_t usable;//是否有buff可用
    uint16_t datasize;// 可以使用数据大小
    uint8_t *pData;
}__mbuf;


__mbuf* CreateMbuf(uint16_t length);
//__mbuf* ResizeMbuf(__mbuf* mp, uint16_t length);











#endif
