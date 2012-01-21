#ifndef __LOCKMUTEX_HPP
#define __LOCKMUTEX_HPP

class LockMutex
{
	mutex_t &m_mutex;
public:
	LockMutex(mutex_t &m) : m_mutex(m) { LWP_MutexLock(m_mutex); }
	~LockMutex(void) { LWP_MutexUnlock(m_mutex); }
};

#endif // !defined(__LOCKMUTEX_HPP)
