#include <Windows.h>
#include "Mutex.h"

struct Mutex::MutexInternal
{
	::CRITICAL_SECTION mtx;
};

Mutex::Mutex(void):m_pInternal(NULL)
{
	m_pInternal = new MutexInternal;
	::InitializeCriticalSection(&m_pInternal->mtx);
}

Mutex::~Mutex(void)
{
	::DeleteCriticalSection(&m_pInternal->mtx);
}

bool Mutex::enter()
{
	::EnterCriticalSection(&m_pInternal->mtx);
	return true;
}

bool Mutex::leave()
{
	::LeaveCriticalSection(&m_pInternal->mtx);
	return true;
}
