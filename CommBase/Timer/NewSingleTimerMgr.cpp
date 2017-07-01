/*
 * NewSingleTimerMgr.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "NewSingleTimerMgr.h"
#include "../Log/Logger.h"
#include <time.h>
#include "../util.h"

namespace CommBaseOut
{

int SingleSlot::AddTimer(Safe_Smart_Ptr<Timer> &timer)
{
	GUARD(CSimLock, obj, &m_lock);
	m_mapTimer.insert( make_pair(timer->m_ID, timer) );

	return 0;
}

list<Safe_Smart_Ptr<Timer> > SingleSlot::OnTick(int64 tick)
{
	int iTimeout = 0;
	list<Safe_Smart_Ptr<Timer> > lTimeoutTimer;			//已超时的循环定时器
	list<Safe_Smart_Ptr<Timer> > lResetTimer;			//需要校正的定时器
	map<DWORD,Safe_Smart_Ptr<Timer> >::iterator itMap;

	//超时处理
	GUARD(CSimLock, obj, &m_lock);
	for( itMap = m_mapTimer.begin(); itMap != m_mapTimer.end();  )
	{
		//超时定时器的处理
		Safe_Smart_Ptr<Timer> pTimer = itMap->second;
		if (pTimer && !pTimer->IsDelete())
		{
			iTimeout = pTimer->OnTickEx(tick);
			if(iTimeout)
			{
				//超时的定时器
//				printf("\n  ontimeout now tick [%lld] tick point[%lld]  slot[%d] distance[%lld]\n", tick, pTimer->m_tickPoint, m_index, pTimer->m_tickPoint-tick);
				lTimeoutTimer.push_back(pTimer);
				m_mapTimer.erase(itMap++);
			}
			else
			{
				if(pTimer->m_tickPoint <= 0)
				{
//					printf("\n round[%d] index[%d] now tick [%lld]\n", pTimer->m_round, m_index, tick);
					pTimer->m_elapseRound = 1;
					pTimer->m_tickPoint = tick;
					++itMap;
				}
				else
				{
					if((tick - pTimer->m_tickPoint) > (pTimer->m_elapseRound * SLOT_CHANGE_TIME * SLOT_COUNT) && ((tick - pTimer->m_tickPoint) % (SLOT_CHANGE_TIME * SLOT_COUNT)) >= SLOT_CHANGE_TIME)
					{
						lResetTimer.push_back(pTimer);
						m_mapTimer.erase(itMap++);
					}
					else
					{
//						printf("\n round[%d] elapse[%u] index[%d] now tick [%lld] tick point[%lld]  distance[%lld]\n", pTimer->m_round, pTimer->m_elapseRound, m_index, tick, pTimer->m_tickPoint, pTimer->m_tickPoint-tick);
						pTimer->m_elapseRound += 1;
						++itMap;
					}
				}
			}
		}
		else
		{
			m_mapTimer.erase(itMap++);
		}
	}

	obj.UnLock();

	std::list<Safe_Smart_Ptr<Timer> >::iterator itList;
	for( itList = lResetTimer.begin(); itList != lResetTimer.end(); ++itList)
	{
		int curslot = ((tick - (*itList)->m_tickPoint) % (SLOT_CHANGE_TIME * SLOT_COUNT)) / SLOT_CHANGE_TIME;

		curslot = m_index - curslot;
		if(curslot < 0)
		{
			curslot += SLOT_COUNT;
		}

//		printf("\n ++++++++++++++++++round[%d] now tick [%lld] tick point[%lld] index[%d] curslot[%d] distance[%lld]+++++++++++++++++++\n", (*itList)->m_round, tick, (*itList)->m_tickPoint, m_index, curslot,  (*itList)->m_tickPoint-tick);
		if(!(*itList)->m_tm->AddTimer(*itList, (DWORD)curslot))
		{
			(*itList)->m_tickPoint += (*itList)->m_elapseRound * SLOT_CHANGE_TIME * SLOT_COUNT;
			(*itList)->m_elapseRound = 1;
		}
		else
		{
			(*itList)->m_elapseRound += 1;
		}
	}

	return lTimeoutTimer;
}

SingleTimerManager::SingleTimerManager():
m_currSlot(0),
m_lastTimerID(0)
{
	for(int i=0; i<SLOT_COUNT; ++i)
	{
		m_slots[i].SetIndex(i);
	}
}

SingleTimerManager::~SingleTimerManager()
{
	End();
}

int SingleTimerManager::svr()
{
	//struct timeval tv;

	int64 beg = 0;
	int64 end = 0;
	int64 tFps = 0;

	while(!m_flag)
	{
		tFps = end - beg - tFps;
		if(tFps < 0)
			tFps = 0;

		tFps = SLOT_CHANGE_TIME - tFps;
		if(tFps < 0)
			tFps = 0;

		beg = (int64)CUtil::GetNowSecond();
		CUtil::MSleep(tFps);

		OnTick(beg+tFps);

		end = (int64)CUtil::GetNowSecond();
	}

	return 0;
}

void SingleTimerManager::Tick(int64 tick)
{
	list<Safe_Smart_Ptr<Timer> > timeoutTimer;

	GUARD(CSimLock, objf, &m_timeoutLock);
	if(m_timeout.size() <= 0)
	{
		return;
	}

	timeoutTimer.insert(timeoutTimer.end(), m_timeout.begin(), m_timeout.end());
	m_timeout.clear();
	objf.UnLock();

	list<Safe_Smart_Ptr<Timer> >::iterator it = timeoutTimer.begin();
	for(; it!=timeoutTimer.end(); ++it)
	{
		if(!(*it)->IsDelete())
			(*it)->OnTimeout();
	}

	it = timeoutTimer.begin();
	for(; it!=timeoutTimer.end(); ++it)
	{
		if(Timer::OnceTimer != (*it)->m_type)
		{
			AttachTimer( *it, tick, 0 );
		}
	}
}

void SingleTimerManager::OnTick(int64 tick)
{
	list<Safe_Smart_Ptr<Timer> > timeoutTimer;
	DWORD slot = 0;
	{
		GUARD(CSimLock, obj, &m_curlock);
		slot = m_currSlot;
		if( ++m_currSlot >= (DWORD)SLOT_COUNT )
			m_currSlot = 0;
		obj.UnLock();
	}

	{
		timeoutTimer = m_slots[slot].OnTick(tick);
		if(timeoutTimer.size() > 0)
		{
			GUARD(CSimLock, objf, &m_timeoutLock);
			m_timeout.insert(m_timeout.end(), timeoutTimer.begin(), timeoutTimer.end());
		}
	}
}

int SingleTimerManager::AddTimer(Safe_Smart_Ptr<Timer> &timer, int isNewTimer)
{
	if (!timer)
		return -1;

	if(timer->m_start <= 0)
	{
		LOG_BASE(FILEINFO, "error param!, timer._start[%lld]", timer->m_start );
		return -1;
	}

	AttachTimer(timer,0,isNewTimer);

	return 0;
}

int SingleTimerManager::AddTimer(Safe_Smart_Ptr<Timer> &timer, DWORD slot)
{
	if(slot >= (DWORD)SLOT_COUNT)
	{
		LOG_BASE(FILEINFO, "examine timer but addtimer error");
		return -1;
	}

	m_slots[slot].AddTimer(timer);

	return 0;
}

void SingleTimerManager::AttachTimer(Safe_Smart_Ptr<Timer> &timer, int64 tick, int isNewTimer )
{
	if (timer == 0 || timer->IsDelete())
		return;

	int slotNum = 0;

	if(isNewTimer)
	{
		timer->m_ID = ++m_lastTimerID;
		if(m_lastTimerID == 0)
		{
			m_lastTimerID = timer->m_ID = 1;
		}

		timer->m_ticks = (timer->m_start - 100)/SLOT_CHANGE_TIME;
	}
	else
	{
		if(timer->m_type == Timer::MonthLoopTimer)
		{
			time_t t;
			struct tm tNow;
			int nmonth = 0;

			time(&t);
			localtime_r(&t, &tNow);

			timer->m_interval = CUtil::GetNextMonthDay(tNow.tm_year + 1900, tNow.tm_mon + 1, tNow.tm_mday, nmonth);
		}

		int64 tSec = (timer->m_interval - tick + timer->m_bePoint);
		if(tSec < 0)
			tSec = 0;

		timer->m_ticks = tSec/SLOT_CHANGE_TIME;
		timer->m_bePoint += timer->m_interval;
		timer->m_tickPoint = 0;
		timer->m_elapseRound = 0;
	}

	timer->m_round = timer->m_ticks/SLOT_COUNT + 1;

	GUARD(CSimLock, obj, &m_curlock);
	slotNum = (m_currSlot + (timer->m_ticks%SLOT_COUNT))%SLOT_COUNT;
	obj.UnLock();

	timer->m_tm = this;

	m_slots[slotNum].AddTimer(timer);
}
}
