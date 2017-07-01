/*
 * TimerInterface.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef TIMER_TIMERINTERFACE_H_
#define TIMER_TIMERINTERFACE_H_

#include "TimerBase.h"
#include "Timer.h"
#include "../Memory/MemAllocator.h"

//定时器用法
/*
TimerConnection 用于接收TimerInterface里面注册的定时器对象，可以通过这个删除定时器，判断定时器是否是合法的

class CTimerFun
{
public:
	CTimerFun()
	{	}

	void TimerFun(void * arg)
	{
		printf("\n TimerFun run \n");
	}
};

///////////////////////////////多线程用法////////////////////////////////////////////////////////////
TimerInterface timer；//例子用一个定义的对象代表用法，真实的是单例
CTimerFun fun;

timer.Init(eTimerThreadSafe);//多线程用法,默认是单线程

TimerConnection timef = timer.RegTimePoint(&CTimerFun::TimerFun, &fun,  0, 20, 52, 10 );//注册在20：52：10执行一次的定时器
timef.SetNull();//这样会删掉timef （20：52：10）定时器，这样就不会执行了

TimerConnection times = timer.RegTimePoint(&CTimerFun::TimerFun, &fun,  0, 0, 1, 3, 1000);//注册距离现在1分3秒1000毫妙后的执行一次的定时器
times由于没有SetNull,times会执行一次

其他定时器都是类型的用法
///////////////////////////////多线程用法////////////////////////////////////////////////////////////



///////////////////////////////单线程用法////////////////////////////////////////////////////////////
TimerInterface timer；//例子用一个定义的对象代表用法，真实的是单例
CTimerFun fun;

timer.Init();//默认是单线程

TimerConnection timef = timer.RegTimePoint(&CTimerFun::TimerFun, &fun,  0, 20, 52, 10 );//注册在20：52：10执行一次的定时器
timef.SetNull();//这样会删掉timef （20：52：10）定时器，这样就不会执行了

TimerConnection times = timer.RegTimePoint(&CTimerFun::TimerFun, &fun,  0, 0, 1, 3, 1000);//注册距离现在1分3秒1000毫妙后的执行一次的定时器
times由于没有SetNull,times会执行一次

while(1)
{
	timer.StartTick(CUtil::GetNowSecond());//单线程必须要用这个，让主循环每一帧都能执行这个
	usleep(100 * 1000);
}

其他定时器都是类型的用法
///////////////////////////////单线程用法////////////////////////////////////////////////////////////
*/

namespace CommBaseOut
{

enum ETimerType
{
	eTimerSingle=0,//���߳�
	eTimerThreadSafe, //���̰߳�ȫ
};

struct TimerConnection
{
	TimerConnection()
	{

	}
	TimerConnection(const Safe_Smart_Ptr<Timer>& timer_con):con(timer_con)
	{

	}

	~TimerConnection()
	{
//		SetNull();
	}

	//销毁
	inline void SetNull()
	{
		if ((bool)con)
		{
			con->SetDelete();
			con = 0;
		}
	}

	inline void operator=(const Safe_Smart_Ptr<Timer>& timer_con)
	{
		if((bool)con && !con->IsDelete())
		{
			con->SetDelete();
			con = 0;
		}

		con = timer_con;
	}

	//是否是有效的
	inline bool IsValid() const
	{
		return (bool)con && !con->IsDelete();
	}

private:
	Safe_Smart_Ptr<Timer> con;
};


class TimerInterface
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:
	TimerInterface(int type):m_timer(0), m_type(type)
	{
	}

	TimerInterface():m_timer(0), m_type(eTimerSingle)
	{
	}

	~TimerInterface()
	{
		if(m_timer)
		{
			delete m_timer;
			m_timer = 0;
		}
	}

	static TimerInterface * GetInstance()
	{
		if (m_instance == 0)
		  {
			m_instance = NEW TimerInterface();
		  }

		return m_instance;
	}

	void DestoryInstance()
	{
		if (m_instance)
		  {
			delete m_instance;
			m_instance = 0;
		  }
	}


	//初始化定时器
	void Init(int type = eTimerSingle);

	//开始定时器运行(单线程最好每一帧都需要进行检测)
	void StartTick(int64 tick = 0);

	void SetDistanceTime(Safe_Smart_Ptr<Timer> & stime, int hour, int minutes, int second, int microSec, int interval = 0);
	void SetDayTime(Safe_Smart_Ptr<Timer> & stime, int hour, int minutes, int second, int interval = 0);
	void SetWeekTime(Safe_Smart_Ptr<Timer> & stime, int weekDay, int hour, int minutes, int second);
	void SetMonthTime(Safe_Smart_Ptr<Timer> & stime, int day, int hour, int minutes, int second);

	//注册某一个时间点执行的定时器(hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒, 只执行一次)
	//@todo hour+minutes+second+microSec > 0
	template<typename T>
	Safe_Smart_Ptr<Timer> RegTimePoint(void (T::*fun)(void * arg), T * obj,  void * arg, int hour, int minutes, int second, int microSec)
	{
		Safe_Smart_Ptr<Timer> newTimer = NEW Timer();

		newTimer->SetCallbackFun<T>(fun, obj, arg);
		newTimer->m_type = Timer::OnceTimer;
		SetDistanceTime(newTimer, hour, minutes, second, microSec);

		return newTimer;
	}
	//注册某一个时间点执行的定时器(hour  minutes  second为第一次执行的时间点时分秒, 只执行一次)
	template<typename T>
	Safe_Smart_Ptr<Timer> RegTimePoint(void (T::*fun)(void * arg), T * obj,  void * arg, int hour, int minutes, int second)
	{
		Safe_Smart_Ptr<Timer> newTimer = NEW Timer();

		newTimer->SetCallbackFun<T>(fun, obj, arg);
		newTimer->m_type = Timer::OnceTimer;
		SetDayTime(newTimer, hour, minutes, second);

		return newTimer;
	}

	//注册循环执行定时器（hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒,  interval 为循环间隔时间，为毫秒）
	//@todo hour+minutes+second+microSec > 0
	template<typename T>
	Safe_Smart_Ptr<Timer> RegRepeatedTime(void (T::*fun)(void * arg), T * obj,  void * arg, int hour, int minutes, int second, int microSec, int interval)
	{
		if(interval < SLOT_CHANGE_TIME)
		{
			interval = SLOT_CHANGE_TIME;
		}

		Safe_Smart_Ptr<Timer> newTimer = NEW Timer();

		newTimer->SetCallbackFun<T>(fun, obj, arg);
		newTimer->m_type = Timer::LoopTimer;
		SetDistanceTime(newTimer, hour, minutes, second, microSec, interval);

		return newTimer;
	}

	//注册循环执行定时器（hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒,  interval 为循环间隔时间，为毫秒）
	//@todo hour+minutes+second+microSec > 0
	template<typename T>
	Safe_Smart_Ptr<Timer> RegDayRepeatedTime(void (T::*fun)(void * arg), T * obj,  void * arg, int hour, int minutes, int second, int microSec)
	{
		Safe_Smart_Ptr<Timer> newTimer = NEW Timer();

		newTimer->SetCallbackFun<T>(fun, obj, arg);
		newTimer->m_type = Timer::LoopTimer;
		SetDistanceTime(newTimer, hour, minutes, second, microSec, 24 * 3600 * 1000);

		return newTimer;
	}
	//注册某一个时间点日循环执行定时器（hour  minutes  second为一天中开始执行的时间点，    23：23：23     每天23点23分23秒执行）
	template<typename T>
	Safe_Smart_Ptr<Timer> RegDayPointRepeatedTime(void (T::*fun)(void * arg), T * obj,  void * arg, int hour, int minutes, int second)
	{
		Safe_Smart_Ptr<Timer> newTimer = NEW Timer();

		newTimer->SetCallbackFun<T>(fun, obj, arg);
		newTimer->m_type = Timer::LoopTimer;
		SetDayTime(newTimer, hour, minutes, second, 24 * 3600 * 1000);

		return newTimer;
	}

	//周循环（hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒）
	//@todo hour+minutes+second+microSec > 0
	template<typename T>
	Safe_Smart_Ptr<Timer> RegWeekRepeatedTime(void (T::*fun)(void * arg), T * obj,  void * arg, int hour, int minutes, int second, int microSec)
	{
		Safe_Smart_Ptr<Timer> newTimer = NEW Timer();

		newTimer->SetCallbackFun<T>(fun, obj, arg);
		newTimer->m_type = Timer::LoopTimer;
		SetDistanceTime(newTimer, hour, minutes, second, microSec, 7 * 24 * 3600 * 1000);

		return newTimer;
	}
	//注册某一个时间点周循环执行定时器（ weekDay  hour  minutes  second 为一周中某一天开始执行的时间点）
	template<typename T>
	Safe_Smart_Ptr<Timer> RegWeekPointRepeatedTime(void (T::*fun)(void * arg), T * obj,  void * arg, int weekDay, int hour, int minutes, int second)
	{
		Safe_Smart_Ptr<Timer> newTimer = NEW Timer();

		newTimer->SetCallbackFun<T>(fun, obj, arg);
		newTimer->m_type = Timer::LoopTimer;
		SetWeekTime(newTimer, weekDay, hour, minutes, second);

		return newTimer;
	}

	//月循环（hour  minutes  second  microSec为第一次执行距离现在的时分秒毫秒,最好是同一天）
	//@todo hour+minutes+second+microSec > 0
	template<typename T>
	Safe_Smart_Ptr<Timer> RegMonthRepeatedTime(void (T::*fun)(void * arg), T * obj,  void * arg, int hour, int minutes, int second, int microSec)
	{
		Safe_Smart_Ptr<Timer> newTimer = NEW Timer();

		newTimer->SetCallbackFun<T>(fun, obj, arg);
		newTimer->m_type = Timer::MonthLoopTimer;
		SetDistanceTime(newTimer, hour, minutes, second, microSec);

		return newTimer;
	}
	//注册某一个时间点月循环执行定时器（ day  hour  minutes  second 为一月中某一天开始执行的时间点）
	template<typename T>
	Safe_Smart_Ptr<Timer> RegMonthPointRepeatedTime(void (T::*fun)(void * arg), T * obj,  void * arg, int day, int hour, int minutes, int second)
	{
		Safe_Smart_Ptr<Timer> newTimer = NEW Timer();

		newTimer->SetCallbackFun<T>(fun, obj, arg);
		newTimer->m_type = Timer::MonthLoopTimer;
		SetMonthTime(newTimer, day, hour, minutes, second);

		return newTimer;
	}

private:
	TimerBase * m_timer;
	int m_type;

	static TimerInterface * m_instance;
};

}

#endif /* TIMER_TIMERINTERFACE_H_ */
