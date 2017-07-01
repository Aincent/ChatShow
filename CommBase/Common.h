/*
 * Common.h
 *
 *  Created on: Jun 8, 2017
 *      Author: root
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <pthread.h>

namespace CommBaseOut
{

#define GUARD(MUTEX, OBJ, LOCK)\
		CommBaseOut::AutoLock<MUTEX> OBJ(LOCK)

#define GUARD_READ(MUTEX, OBJ, LOCK) \
		CommBaseOut::AutoRWLock<MUTEX> OBJ(LOCK);\
		OBJ.ReadLock()

#define GUARD_WRITE(MUTEX, OBJ, LOCK) \
		CommBaseOut::AutoRWLock<MUTEX> OBJ(LOCK);\
		OBJ.WriteLock()

template<class T>
class AutoLock
{
public:
	AutoLock(T *l):m_lock(l),m_flag(false)
	{
		Lock();
	}

	void Lock()
	{
		m_flag = true;
		m_lock->Lock();
	}

	void UnLock()
	{
		if(m_flag)
		{
			m_lock->UnLock();
			m_flag = false;
		}
	}

	~AutoLock()
	{
		UnLock();
	}

private:
	T *m_lock;
	bool m_flag;
};


class CSimLock
{
public:
	CSimLock()
	{
		pthread_mutex_init(&m_mutex, NULL);
	}

	void Lock()
	{
		pthread_mutex_lock(&m_mutex);
	}

	void UnLock()
	{
		pthread_mutex_unlock(&m_mutex);
	}

	~CSimLock()
	{
		pthread_mutex_destroy(&m_mutex);
	}

private:
	pthread_mutex_t m_mutex;
};

class CQuickLock
{
public:
	CQuickLock()
	{
		pthread_spin_init(&m_mutex, 0);
	}

	void Lock()
	{
		pthread_spin_lock(&m_mutex);
	}

	void UnLock()
	{
		pthread_spin_unlock(&m_mutex);
	}

	~CQuickLock()
	{
		pthread_spin_destroy(&m_mutex);
	}

private:
	pthread_spinlock_t m_mutex;
};


template<class T>
class AutoRWLock
{
public:
	AutoRWLock(T *l):m_lock(l),m_flag(false)
	{
	}

	void ReadLock()
	{
		m_flag = true;
		m_lock->ReadLock();
	}

	void WriteLock()
	{
		m_flag = true;
		m_lock->WriteLock();
	}

	void UnLock()
	{
		if(m_flag)
		{
			m_lock->UnLock();
			m_flag = false;
		}
	}

	~AutoRWLock()
	{
		UnLock();
	}

private:
	T *m_lock;
	bool m_flag;
};

class CRWLock
{
public:
	CRWLock()
	{
		pthread_rwlock_init(&m_mutex, 0);
	}

	void ReadLock()
	{
		pthread_rwlock_rdlock(&m_mutex);
	}

	void WriteLock()
	{
		pthread_rwlock_wrlock(&m_mutex);
	}

	void UnLock()
	{
		pthread_rwlock_unlock(&m_mutex);
	}

	~CRWLock()
	{
		pthread_rwlock_destroy(&m_mutex);
	}

private:
	pthread_rwlock_t m_mutex;
};

class Event
{
public :
	Event()
	{
		pthread_mutex_init(&m_mutex, NULL);
		pthread_cond_init(&m_cond, NULL);
	}
	~Event()
	{
		pthread_cond_destroy(&m_cond);
		pthread_mutex_destroy(&m_mutex);
	}

	int WaitForSingleEvent()
	{
		int res = 0;

		pthread_mutex_lock(&m_mutex);
		res = pthread_cond_wait(&m_cond, &m_mutex);
		pthread_mutex_unlock(&m_mutex);

		return res;
	}

	int SetEvent()
	{
		int res = 0;

		pthread_mutex_lock(&m_mutex);
		res = pthread_cond_signal(&m_cond);
		pthread_mutex_unlock(&m_mutex);

		return res;
	}

private :

	pthread_mutex_t m_mutex;
	pthread_cond_t  m_cond;
};

}

#endif /* COMMON_H_ */
