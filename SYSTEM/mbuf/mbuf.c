
#include "mbuf.h"

#include "stdlib.h"

__mbuf* CreateMbuf(uint16_t length)
{
    __mbuf *p=NULL;
    p = (__mbuf*)malloc(length);// + sizeof(__mbuf));
    if(p==NULL)
        return p;
    p->pNext=NULL;
    p->usable=0;
//    p->packed=0;
    p->pData=(uint8_t*)p+sizeof(__mbuf);
    return p;
}














