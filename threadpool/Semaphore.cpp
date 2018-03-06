#include "Semaphore.h"
#include <assert.h>
#define  INCREMENT_AMOUT 1

#ifdef _MSC_VER
#include <Windows.h>
#endif // _MSC_VER

#ifndef NULL
#define NULL    0
#endif // !NULL

#ifndef LONG_MAX
#define LONG_MAX 2147483647l  /* maxinum (signed) ling value*/
#endif // !LONG_MAX


struct Semaphore::SemaphoreInternal
{
	void* handle;
};


Semaphore::Semaphore(int initialCount)
{
	m_internal = new SemaphoreInternal;
	m_internal->handle = CreateSemaphore(NULL,initialCount,LONG_MAX,NULL);
	assert(m_internal->handle);
}

Semaphore::~Semaphore(void)
{
	CloseHandle(m_internal->handle);
	delete m_internal;
}

int Semaphore::pend()
{
	assert(m_internal->handle);
	return WaitForSingleObject(m_internal->handle,INFINITE);
}

int Semaphore::pend(unsigned long timeout)
{
	assert(m_internal->handle);
	DWORD ret = WaitForSingleObject(m_internal->handle,timeout);
	if (ret == WAIT_OBJECT_0)
		return 0;
	return -1;
}

int Semaphore::post()
{
	LONG cnt = 0;
	// 获取当前信号量被使用的数量
	return ReleaseSemaphore(m_internal->handle,INCREMENT_AMOUT,&cnt);
}