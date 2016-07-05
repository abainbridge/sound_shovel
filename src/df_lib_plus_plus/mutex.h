#ifndef INCLUDED_MUTEX_H
#define INCLUDED_MUTEX_H


class Mutex
{
private:
    void *m_mutexData;

public:
    Mutex();
    ~Mutex();
    void Enter();
    void Leave();
};


// This wrapper class makes
class MutexLocker
{
private:
    Mutex *m_mutex;

public:
    MutexLocker(Mutex *mutex)
    {
        mutex->Enter();
        m_mutex = mutex;
    }

    ~MutexLocker()
    {
        m_mutex->Leave();
    }
};


#endif
