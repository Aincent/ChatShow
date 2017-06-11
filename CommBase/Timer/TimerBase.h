/*
 * TimerBase.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef TIMER_TIMERBASE_H_
#define TIMER_TIMERBASE_H_

#include "Timer.h"
#include "../Smart_Ptr.h"
#include "../Memory/MemAllocator.h"

namespace CommBaseOut
{

class TimerBase
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:
	TimerBase(){}
	virtual ~TimerBase(){}

	virtual bool start(TimerBase* tm)
	{
		return true;
	}

	virtual int AddTimer(Safe_Smart_Ptr<Timer> &timer, int isNewTimer = 1)
	{
		return 0;
	}

	virtual int AddTimer(Safe_Smart_Ptr<Timer> &timer, DWORD slot)
	{
		return 0;
	}

private:
};

}

#endif /* TIMER_TIMERBASE_H_ */
