// Own header
#include "threading.h"


CriticalSection::CriticalSection()
{
	InitializeCriticalSection(&m_criticalSectionWin32);
}


CriticalSection::~CriticalSection()
{
	DeleteCriticalSection(&m_criticalSectionWin32);
}


void CriticalSection::Enter(char const *owner)
{
	EnterCriticalSection(&m_criticalSectionWin32);
    m_owner = owner;
}


void CriticalSection::Leave()
{
	LeaveCriticalSection(&m_criticalSectionWin32);
    m_owner = NULL;
}


unsigned StartThread(ThreadProc threadFunc, void *threadData)
{
	DWORD threadId;
	HANDLE threadHandle = CreateThread(NULL, 0, threadFunc, threadData, 0, &threadId);
    return (unsigned)threadHandle;
}


bool MySuspendThread(unsigned threadHandle)
{
	unsigned long result = SuspendThread((HANDLE)threadHandle);
	return result != 0xFFFFFFFF;
}


bool MyResumeThread(unsigned threadHandle)
{
	unsigned long result = ResumeThread((HANDLE)threadHandle);
	return result != 0xFFFFFFFF;
}

