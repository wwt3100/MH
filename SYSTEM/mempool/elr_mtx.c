#include "elr_mtx.h"

#ifdef   WINDOWS

elr_counter_t elr_atomic_inc(elr_atomic_t* v)
{
	return InterlockedIncrement(v);
}

elr_counter_t elr_atomic_dec(elr_atomic_t* v)
{
	return InterlockedDecrement(v);
}

/*
** ��ʼ�������壬����0��ʾ��ʼ��ʧ��
*/
int  elr_mtx_init(elr_mtx *mtx)
{
	DWORD err;
	InitializeCriticalSection(&mtx->_cs);
	err = GetLastError();
	if (STATUS_NO_MEMORY == err)
	{
		DeleteCriticalSection(&mtx->_cs);
		return 0;
	}

	return 1;
}

void elr_mtx_lock (elr_mtx *mtx)
{
	EnterCriticalSection(&mtx->_cs);
}

void elr_mtx_unlock(elr_mtx *mtx)
{
	LeaveCriticalSection(&mtx->_cs);
}

void elr_mtx_finalize(elr_mtx *mtx)
{
	DeleteCriticalSection(&mtx->_cs);
}
#endif