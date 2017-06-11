/*
 * Timer.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef TIMER_TIMER_H_
#define TIMER_TIMER_H_

#include "../define.h"
#include "../Ref_Object.h"
#include "../Memory/MemAllocator.h"
#include "../Common.h"
#include "../Smart_Ptr.h"

namespace CommBaseOut
{

class TimerSlotBase
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:
	virtual ~TimerSlotBase() {};
	virtual void operator()() = 0;
};

template<typename T>
class TimerFunctionSlot : public TimerSlotBase
{
public:
	typedef void (T::*TIMER_CALLBACK_FUN)(void * arg);

	TimerFunctionSlot(TIMER_CALLBACK_FUN func, T* obj, void * arg) :
	m_function(func),m_object(obj),	m_arg(arg)
	{}

	virtual void operator()()
	{
		(m_object->*m_function)(m_arg);
	}

private:
	TIMER_CALLBACK_FUN m_function;
	T* m_object;
	void * m_arg;
};

class TimerBase;

class Timer : public Ref_Object
#ifdef USE_MEMORY_POOL
, public MemoryBase
#endif
{
public:
	friend class TimerManager;

public:
	enum TimerType
	{
		LoopTimer,
		OnceTimer,
		MonthLoopTimer,
	};

	Timer();
	~Timer();

	template<typename T>
	void SetCallbackFun(void (T::*function)(void * arg), T * obj,  void * arg)
	{
		m_obj = NEW TimerFunctionSlot<T>(function, obj, arg);
	}

	int OnTick(int64 tick);
	int OnTickEx(int64 tick);
	void OnTimeout();
	bool Cancel();
	bool IsRunning() {return m_tm != 0;}
	bool IsDelete();
	void SetDelete();

public:

	TimerSlotBase * m_obj;
	TimerType m_type;
	DWORD m_ID;
	int64 m_interval;
	DWORD m_start;
	int64 m_bePoint;
	int64 m_tickPoint;
	WORD m_elapseRound;

	CSimLock m_delLock;
	bool m_delflag; //是否删除
	DWORD m_ticks;
	int m_round;
	TimerBase* m_tm;
};
}

#endif /* TIMER_TIMER_H_ */
