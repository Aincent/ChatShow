/*
 * ThreadPool.cpp
 *
 *  Created on: Jun 10, 2017
 *      Author: root
 */
#include "ThreadPool.h"
#include "../util.h"

namespace CommBaseOut
{

CThreadPool * CThreadPool::m_instance = 0;

CThreadPool::CThreadPool():m_dwThreadNum(0),m_bExitThread(false),m_pID(0)
{
}

CThreadPool::~CThreadPool()
{
	if(m_pID)
	{
		DELETE_BASE(m_pID, eMemoryArray);
		m_pID = 0;
	}
}

void CThreadPool::InitThread(DWORD dwThreadNum)
{
	m_pID = NEW_BASE(pthread_t, dwThreadNum); // pthread_t[num];
	m_dwThreadNum = dwThreadNum;
	for(DWORD i = 0; i < dwThreadNum; i++)
    {
		pthread_create(&m_pID[i], NULL, &ThreadWork, 0);
    }
}

void CThreadPool::AddTask(void *(*pFunThread)(void *), void *arg)
{
	struct tagThreadInfo threadInfo;
	threadInfo.pFun = pFunThread;
	threadInfo.arg = arg;
	GUARD(CSimLock, obj, &m_mutexTask);
	m_queueTask.push(threadInfo);
	obj.UnLock();

	m_event.SetEvent();
}

void *CThreadPool::ThreadWork(void *arg)
{
	while(!m_instance->m_bExitThread)
	{
		GUARD(CSimLock, obj, &m_instance->m_mutexTask);
		if(!m_instance->m_queueTask.empty())
        {
			struct tagThreadInfo threadInfo = m_instance->m_queueTask.front();
			m_instance->m_queueTask.pop();
			obj.UnLock();

			if(threadInfo.pFun)
				threadInfo.pFun(threadInfo.arg);
        }
		else
        {
			obj.UnLock();

			m_instance->m_event.WaitForSingleEvent();
        }
	}

	return NULL;
}

void CThreadPool::DestroyInstance()
{
	m_bExitThread = true;
	void *status = 0;
	for(unsigned int i=0; i<m_dwThreadNum; ++i)
	{
		m_event.SetEvent();
		pthread_join(m_pID[i], &status);
	}

	if(m_instance)
	{
		delete m_instance;
	}
}

}
