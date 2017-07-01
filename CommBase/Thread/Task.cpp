/*
 * Task.cpp
 *
 *  Created on: Jun 10, 2017
 *      Author: root
 */
#include "Task.h"
#include <stdlib.h>
#include "../Log/Logger.h"
#include "../Memory/MemAllocator.h"

namespace CommBaseOut
{

Task::Task():m_flag(false),m_count(0),m_pID(0)
{

}

Task::~Task()
{
	if(m_pID)
	{
		DELETE_BASE(m_pID, eMemoryArray);
		m_pID = 0;
	}
}

int Task::Start(int num, pthread_attr_t *attr)
{
	m_count = num;
	m_pID = NEW_BASE(pthread_t, num); // pthread_t[num];
//	m_pID = (pthread_t *)NEW_BASE(pthread_t, num); // pthread_t[num];
	m_flag = false;

	for(int i=0; i<num; ++i)
	{
		if(pthread_create(&m_pID[i], attr, ThreadRun, this))
		{
			LOG_BASE(FILEINFO, "pthread_create failed");

			return eCreateThreadErr;
		}
	}

	return eTaskSuccess;
}

int Task::End()
{
	if(!m_flag)
	{
		m_flag = true;

		void *status = 0;
		for(int i=0; i<m_count; ++i)
		{
			pthread_join(m_pID[i], &status);
		}

		if(m_pID)
		{
			DELETE_BASE(m_pID, eMemoryArray);
			m_pID = 0;
		}
	}

	return eTaskSuccess;
}

void *Task::ThreadRun(void *p)
{
	Task *task = static_cast<Task *>(p);

	task->svr();

	return 0;
}

int Task::svr()
{
	return eTaskSuccess;
}
}
