#ifndef INCLUDED_THREADING_H
#define INCLUDED_THREADING_H


#include <windows.h>    // TODO make this not needed in the header by making m_criticalSectionWin32 be a pointer


class CriticalSection
{
private:
	CRITICAL_SECTION m_criticalSectionWin32;
    char const *m_owner;

public:
	CriticalSection();
	~CriticalSection();

    char const *GetOwner() { return m_owner; }
	void Enter(char const *owner);
	void Leave();
};


typedef unsigned long (__stdcall *ThreadProc)(void *data);

// Returns a thread handle, or zero on failure
unsigned StartThread(ThreadProc threadFunc, void *threadData);
bool MySuspendThread(unsigned threadHandle);	// Returns true on success
bool MyResumeThread(unsigned threadHandle);		// Returns true on success


#endif
