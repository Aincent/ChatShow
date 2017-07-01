/*
 * TimerInterface.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "TimerInterface.h"
#include "NewSingleTimerMgr.h"
#include "TimerManager.h"
#include "../Smart_Ptr.h"
#include "../util.h"
#include <time.h>
#include "../Log/Logger.h"

namespace CommBaseOut
{

TimerInterface * TimerInterface::m_instance = 0;

void TimerInterface::Init(int type)
{
	m_type = type;

	if(m_type == eTimerSingle)
	{
		m_timer = NEW SingleTimerManager();
		static_cast<SingleTimerManager *>(m_timer)->Start(1);
	}
	else
	{
		m_timer = NEW TimerManager();
		static_cast<TimerManager *>(m_timer)->Start(1);
	}
}

void TimerInterface::StartTick(int64 tick)
{
	if(m_type == eTimerSingle)
	{
		static_cast<SingleTimerManager *>(m_timer)->Tick(tick);
	}
}

void TimerInterface::SetDistanceTime(Safe_Smart_Ptr<Timer> & stime, int hour, int minutes, int second, int microSec, int interval)
{
	stime->m_start = (int64)hour * 3600 * 1000 + (int64)minutes * 60 * 1000 + (int64)second * 1000 + (int64)microSec;
	if(stime->m_start < 100)
		stime->m_start = 100;
	stime->m_bePoint = CUtil::GetNowSecond() + stime->m_start;
	stime->m_interval = interval;

	if(m_timer->AddTimer(stime))
	{
		stime = 0;
	}
}

void TimerInterface::SetDayTime(Safe_Smart_Ptr<Timer> & stime, int hour, int minutes, int second, int interval)
{
	int64 diftime = 0;
	time_t t;
	struct tm tNow;

	time(&t);
	localtime_r(&t, &tNow);
	diftime = ((int64)hour - (int64)tNow.tm_hour) * 3600 * 1000 + ((int64)minutes - (int64)tNow.tm_min) * 60 * 1000 + ((int64)second - (int64)tNow.tm_sec) * 1000;
	if(diftime < 0)
	{
		diftime += 24 * 3600 * 1000;
	}

	stime->m_start = diftime;
	if(stime->m_start < 100)
		stime->m_start = 100;
	stime->m_bePoint = CUtil::GetNowSecond() + stime->m_start;
	stime->m_interval = interval;

	if(m_timer->AddTimer(stime))
	{
		stime = 0;
	}
}

void TimerInterface::SetWeekTime(Safe_Smart_Ptr<Timer> & stime, int weekDay, int hour, int minutes, int second)
{
	int64 diftime = 0;
	time_t t;
	struct tm tNow;
	int wday = 0;

	time(&t);
	localtime_r(&t, &tNow);
	wday = tNow.tm_wday;
	if(wday == 0)
		wday = 7;

	diftime = ((int64)weekDay - (int64)wday) * 24 * 3600 * 1000 + ((int64)hour - (int64)tNow.tm_hour) * 3600 * 1000 + ((int64)minutes - (int64)tNow.tm_min) * 60 * 1000 + ((int64)second - (int64)tNow.tm_sec) * 1000;
	if(diftime < 0)
	{
		diftime += 7 *  24 * 3600 * 1000;
	}

	stime->m_start = diftime;
	if(stime->m_start < 100)
		stime->m_start = 100;
	stime->m_bePoint = CUtil::GetNowSecond() + stime->m_start;
	stime->m_interval = 7 * 24 * 3600 * 1000;

	if(m_timer->AddTimer(stime))
	{
		stime = 0;
	}
}

void TimerInterface::SetMonthTime(Safe_Smart_Ptr<Timer> & stime, int day, int hour, int minutes, int second)
{
	int64 diftime = 0;
	time_t t;
	struct tm tNow;

	time(&t);
	localtime_r(&t, &tNow);
	diftime = ((int64)day - (int64)tNow.tm_mday) * 24 * 3600 * 1000 + ((int64)hour - (int64)tNow.tm_hour) * 3600 * 1000 + ((int64)minutes - (int64)tNow.tm_min) * 60 * 1000 + ((int64)second - (int64)tNow.tm_sec) * 1000;
	if(diftime < 0)
	{
		int nmonth = 0;
		diftime += CUtil::GetNextMonthDay(tNow.tm_year + 1900, tNow.tm_mon + 1, tNow.tm_mday, nmonth);
	}

	stime->m_start = diftime;
	if(stime->m_start < 100)
		stime->m_start = 100;
	stime->m_bePoint = CUtil::GetNowSecond() + stime->m_start;

	if(m_timer->AddTimer(stime))
	{
		stime = 0;
	}
}
}
