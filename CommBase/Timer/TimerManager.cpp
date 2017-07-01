/*
 * TimerManager.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "TimerManager.h"
#include "../Log/Logger.h"
#include <time.h>
#include "../util.h"

namespace CommBaseOut
{

int Slot::AddTimer(Safe_Smart_Ptr<Timer> &timer)
{
	GUARD(CSimLock, obj, &m_lock);
	m_mapTimer.insert( make_pair(timer->m_ID, timer) );

	return 0;
}

list<Safe_Smart_Ptr<Timer> > Slot::OnTick(int64 tick)
{
	int iTimeout = 0;
	list<Safe_Smart_Ptr<Timer> >	lTimeoutTimerAll;				//所有已超时的定时器
	list<Safe_Smart_Ptr<Timer> > lTimeoutTimerLoop;			//已超时的循环定时器
	map<DWORD,Safe_Smart_Ptr<Timer> >::iterator itMap;

	//超时处理
	GUARD(CSimLock, obj, &m_lock);
	for( itMap = m_mapTimer.begin(); itMap != m_mapTimer.end(); )
	{
		//超时定时器的处理
		Safe_Smart_Ptr<Timer> pTimer = itMap->second;
		if (pTimer && !pTimer->IsDelete())
		{
			//超时的定时器
			lTimeoutTimerAll.push_back(pTimer);
		}
		else
		{
			m_mapTimer.erase(itMap++);
			continue;
		}

		++itMap;
	}
	obj.UnLock();

	std::list<Safe_Smart_Ptr<Timer> >::iterator itList;
	for( itList = lTimeoutTimerAll.begin(); itList != lTimeoutTimerAll.end(); )
	{
		iTimeout = (*itList)->OnTick(tick);
		if(iTimeout)
		{
			//超时的循环定时器
//			printf("\n  ontimeout now tick [%lld]  slot[%d]\n", tick, m_index);
//			printf("\n  ontimeout now tick [%lld] tick point[%lld]  slot[%d] distance[%lld]\n", tick, (*itList)->m_tickPoint, m_index, (*itList)->m_tickPoint-tick);
			if(Timer::OnceTimer != (*itList)->m_type)
			{
				lTimeoutTimerLoop.push_back(*itList);
			}
			else
			{
				(*itList)->SetDelete();
			}

			++itList;
		}
		else
		{
			Safe_Smart_Ptr<Timer> pppt = (*itList);
			if((*itList)->m_tickPoint <= 0)
			{
				(*itList)->m_tickPoint  = tick;
				(*itList)->m_elapseRound = 1;
//				printf("\n round[%d] index[%d] now tick [%lld]\n", (*itList)->m_round, m_index, tick);
//				printf("\n id[%u] slot[%d] tick point[%lld] round[%d] tick[%u] start[%u interval[%u]\n", (*itList)->m_ID, m_index, tick, (*itList)->m_round, (*itList)->m_ticks, (*itList)->m_start, (*itList)->m_interval);
				lTimeoutTimerAll.erase(itList++);
			}
			else
			{
				if((tick - (*itList)->m_tickPoint) > ((*itList)->m_elapseRound * SLOT_CHANGE_TIME * SLOT_COUNT) && ((tick - (*itList)->m_tickPoint) % (SLOT_CHANGE_TIME * SLOT_COUNT)) >= SLOT_CHANGE_TIME)
				{
					int curslot = ((tick - (*itList)->m_tickPoint) % (SLOT_CHANGE_TIME * SLOT_COUNT)) / SLOT_CHANGE_TIME;

					curslot = m_index - curslot;
					if(curslot < 0)
					{
//						(*itList)->m_round -= 1;
						curslot += SLOT_COUNT;
//						printf("\n-----------------------------------------------------------------------------------\n");
					}

//					printf("\n ++++++++++++++++++round[%d] now tick [%lld] tick point[%lld] index[%d] curslot[%d] distance[%lld]+++++++++++++++++++\n", (*itList)->m_round, tick, (*itList)->m_tickPoint, m_index, curslot,  (*itList)->m_tickPoint-tick);
//					printf("\n id[%u] slot[%d] newslot[%d] tick point[%lld] round[%d] tick[%u] start[%u interval[%u]\n", (*itList)->m_ID, m_index, curslot, tick, (*itList)->m_round, (*itList)->m_ticks, (*itList)->m_start, (*itList)->m_interval);
					if((*itList)->m_tm->AddTimer(*itList, (DWORD)curslot))
					{
						(*itList)->m_elapseRound += 1;
						lTimeoutTimerAll.erase(itList++);
					}
					else
					{
						(*itList)->m_tickPoint += (*itList)->m_elapseRound * SLOT_CHANGE_TIME * SLOT_COUNT;
						(*itList)->m_elapseRound = 1;
						++itList;
					}
				}
				else
				{
//					printf("\n round[%d] elapse[%u] index[%d] now tick [%lld] tick point[%lld]  distance[%lld]\n", (*itList)->m_round, (*itList)->m_elapseRound, m_index, tick, (*itList)->m_tickPoint, (*itList)->m_tickPoint-tick);
//					printf("\n   now tick [%lld]  slot[%d]\n", tick, m_index);

					(*itList)->m_elapseRound += 1;
					lTimeoutTimerAll.erase(itList++);
				}
			}
		}
	}

	//清理所有超时的定时器
	GUARD(CSimLock, obj1, &m_lock);
	for( itList = lTimeoutTimerAll.begin(); itList != lTimeoutTimerAll.end(); itList++ )
	{
		itMap = m_mapTimer.find((*itList)->m_ID);
		if( itMap != m_mapTimer.end() )
		{
			m_mapTimer.erase(itMap);
		}
	}

	return lTimeoutTimerLoop;
}

TimerManager::TimerManager():
m_currSlot(0),
m_lastTimerID(0)
{
	for(int i=0; i<SLOT_COUNT; ++i)
	{
		m_slots[i].SetIndex(i);
	}
}

TimerManager::~TimerManager()
{
	End();
}

int TimerManager::svr()
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

void TimerManager::OnTick(int64 tick)
{
	list<Safe_Smart_Ptr<Timer> > lRetsetTimer;
	int curslot = 0;

	GUARD(CSimLock, obj, &m_curlock);
	curslot = m_currSlot;
	if( ++m_currSlot >= (DWORD)SLOT_COUNT )
		m_currSlot = 0;
	obj.UnLock();

	lRetsetTimer = m_slots[curslot].OnTick(tick);

	list<Safe_Smart_Ptr<Timer> >::iterator it;
	for( it = lRetsetTimer.begin(); it != lRetsetTimer.end(); it++ )
	{
		AttachTimer( *it, tick, 0 );
	}
}

int TimerManager::AddTimer(Safe_Smart_Ptr<Timer> &timer, int isNewTimer)
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

int TimerManager::AddTimer(Safe_Smart_Ptr<Timer> &timer, DWORD slot)
{
	if(slot >= (DWORD)SLOT_COUNT)
		return -1;

	m_slots[slot].AddTimer(timer);

	return 0;
}

void TimerManager::AttachTimer(Safe_Smart_Ptr<Timer> &timer, int64 tick, int isNewTimer )
{
	if (timer == 0 || timer->IsDelete())
		return;

	int slotNum = 0;

	if(isNewTimer)
	{
		GUARD(CSimLock, obj, &m_lock);
		timer->m_ID = ++m_lastTimerID;
		if(m_lastTimerID == 0)
		{
			m_lastTimerID = timer->m_ID = 1;
		}
		obj.UnLock();

		timer->m_ticks = timer->m_start/SLOT_CHANGE_TIME;
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
