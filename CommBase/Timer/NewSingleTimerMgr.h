/*
 * NewSingleTimerMgr.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef TIMER_NEWSINGLETIMERMGR_H_
#define TIMER_NEWSINGLETIMERMGR_H_

#include "Timer.h"
#include <list>
#include "../Common.h"
#include "../Thread/Task.h"
#include "TimerBase.h"
#include "TimerManager.h"

using namespace std;

namespace CommBaseOut
{

class SingleSlot
{
public:
	void SetIndex(int i) { 	m_index = i; }
	int	AddTimer(Safe_Smart_Ptr<Timer> &timer);
	list<Safe_Smart_Ptr<Timer> > OnTick(int64 tick);

private:

	CSimLock m_lock;

	map<DWORD, Safe_Smart_Ptr<Timer> > m_mapTimer; //Guarded by mutex_
	int m_index;
};


class SingleTimerManager : public Task, public TimerBase
{
public:
	SingleTimerManager();
	~SingleTimerManager();

	virtual int svr();
	void Tick(int64 tick);
	void OnTick(int64 tick);
	virtual int AddTimer(Safe_Smart_Ptr<Timer> &timer, int isNewTimer = 1);
	virtual int AddTimer(Safe_Smart_Ptr<Timer> &timer, DWORD slot);

private:
	void AttachTimer( Safe_Smart_Ptr<Timer> &timer, int64 tick, int isNewTimer );

private:

	SingleSlot m_slots[SLOT_COUNT];

	DWORD	m_currSlot;
	CSimLock m_curlock;

	DWORD m_lastTimerID;

	list<Safe_Smart_Ptr<Timer> > m_timeout;
	CSimLock m_timeoutLock;
};
}

#endif /* TIMER_NEWSINGLETIMERMGR_H_ */
