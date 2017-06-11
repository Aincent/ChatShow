/*
 * TimerManager.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef TIMER_TIMERMANAGER_H_
#define TIMER_TIMERMANAGER_H_

#include "Timer.h"
#include <list>
#include "../Common.h"
#include "../Thread/Task.h"
#include "TimerBase.h"

using namespace std;

namespace CommBaseOut
{

class Slot
{
public:
	void SetIndex(int i) { 	m_index = i; }
	int	AddTimer(Safe_Smart_Ptr<Timer> &timer);
	list<Safe_Smart_Ptr<Timer> > OnTick(int64 tick);

private:

	CSimLock m_lock;
	map<DWORD, Safe_Smart_Ptr<Timer> > m_mapTimer;
	int m_index;
};

class TimerManager : public Task, public TimerBase
{
public:
	TimerManager();
	~TimerManager();
	virtual int svr();
	void OnTick(int64 tick);
	virtual int AddTimer(Safe_Smart_Ptr<Timer> &timer, int isNewTimer = 1);
	virtual int AddTimer(Safe_Smart_Ptr<Timer> &timer, DWORD slot);

private:
	void AttachTimer( Safe_Smart_Ptr<Timer> &timer, int64 tick, int isNewTimer );

private:

	Slot m_slots[SLOT_COUNT];

	DWORD	m_currSlot;
	CSimLock m_curlock;

	DWORD m_lastTimerID;
	CSimLock m_lock;
};
}

#endif /* TIMER_TIMERMANAGER_H_ */
