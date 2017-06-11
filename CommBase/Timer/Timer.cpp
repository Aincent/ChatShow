/*
 * Timer.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "Timer.h"

namespace CommBaseOut
{

Timer::Timer():m_obj(0),m_type(OnceTimer),m_ID(0),m_interval(0),m_start(0),m_bePoint(0),m_tickPoint(0),m_elapseRound(0),m_delflag(false),m_ticks(0),m_round(0),m_tm(0)
{
}

Timer::~Timer()
{
	if(m_obj)
	{
		delete m_obj;
		m_obj = 0;
	}

}

int Timer::OnTick(int64 tick)
{
	int iTimeout = 0;

	if(tick - m_bePoint >= 0 || --m_round <= 0)
	{
		iTimeout = 1;
		OnTimeout();
	}

	return iTimeout;
}

int Timer::OnTickEx(int64 tick)
{
	int iTimeout = 0;

	if(tick - m_bePoint >= 0 || --m_round <= 0)
	{
		iTimeout = 1;
	}

	return iTimeout;
}

bool Timer::IsDelete()
{
	GUARD(CSimLock, obj, &m_delLock);
//	printf("\n ------------------ exame timer[%d]  this[%p] ------------------\n", m_ID, this);
	return m_delflag;
}

void Timer::SetDelete()
{
//	printf("\n +++++++++++++++++++++    delete  timer[%d]  this[%p]  ++++++++++++\n", m_ID, this);
	GUARD(CSimLock, obj, &m_delLock);
	m_delflag = true;
}

void Timer::OnTimeout()
{
	(*m_obj)();
}

bool Timer::Cancel()
{
	GUARD(CSimLock, obj, &m_delLock);
	m_delflag = true;

	return true;
}

}
