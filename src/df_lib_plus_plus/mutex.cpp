#include "mutex.h"

#include <windows.h>


#ifdef _MSC_VER


Mutex::Mutex()
{
    CRITICAL_SECTION *cs = new CRITICAL_SECTION;
    m_mutexData = (void*)cs;
    InitializeCriticalSection(cs);
}


Mutex::~Mutex()
{
    DeleteCriticalSection((CRITICAL_SECTION*)m_mutexData);
}


void Mutex::Enter()
{
    EnterCriticalSection((CRITICAL_SECTION*)m_mutexData);
}


void Mutex::Leave()
{
    LeaveCriticalSection((CRITICAL_SECTION*)m_mutexData);
}


#endif
