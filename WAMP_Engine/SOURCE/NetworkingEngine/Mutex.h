// Mutex.h: Interface for working with mutexes.
// CS260 Assignment 2
// Feb 25th 2010

#pragma once

class Lock;  // forward declare for mutex
class Mutex
{
public:
	Mutex () { InitializeCriticalSection (&CriticalSection); }
	~Mutex () { DeleteCriticalSection (&CriticalSection); }

private:
	// Acquire and release will be called by the lock only, there is no
	// external access.
	void Acquire ()
	{
		EnterCriticalSection (&CriticalSection);
	}

	void Release ()
	{
		LeaveCriticalSection (&CriticalSection);
	}

	CRITICAL_SECTION CriticalSection;  // the critical section for this mutex

	friend class Lock;
};


class Lock
{
public:
	// Acquire the semaphore
	Lock ( Mutex& mutex_ )
		: mutex(mutex_)
	{
		mutex.Acquire();
	}

	// Release the semaphore
	~Lock ()
	{
		mutex.Release();
	}

private:
	Mutex& mutex;
};
