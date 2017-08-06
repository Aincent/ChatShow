/*
 * EventDefine.h
 *
 *  Created on: Jul 22, 2017
 *      Author: root
 */

#ifndef EVENTDEFINE_H_
#define EVENTDEFINE_H_

#include "GlobalEvent.h"

//事件的用法
/*

class CTestEvent : public Singleton<CTestEvent>
{
public:
	CTestEvent()
	{
	}

	~CTestEvent()
	{
	}

	bool TestEvetn(const EventArgs & e)
	{
		printf("\n\n------11111---&&&&&&\n\n");
	}

	bool TestEvetn2(const EventArgs & e)
	{
		printf("\n\n-----222222----&&&&&&\n\n");
	}


};

class LocalEvent : public EventSet
{
public:


};

class TestArgs : public EventArgs
{
public:

	TestArgs(int i):m_id(i)
	{

	}

	int m_id;
};

	EventConnection connectionE1;

	//注册全局事件1
	connectionE1 = RegGlobalEventPtr(1, CTestEvent::GetInstance(), &CTestEvent::TestEvetn);

	EventConnection connectionE2;

	//注册全局事件1
	connectionE2 = RegGlobalEventPtr(1, CTestEvent::GetInstance(), &CTestEvent::TestEvetn2);

	TestArgs tArgs(1);

	//触发全局事件会回调所有的注册了的事件
	FireGlobalEvent(1,tArgs);//执行CTestEvent::TestEvetn 和 CTestEvent::TestEvetn2

	connectionE1.SetNull();//销毁事件1

	TestArgs tArgs1(1);

	//销毁了一个事件，触发事件2
	FireGlobalEvent(1,tArgs1);//执行CTestEvent::TestEvetn2

	EventConnection connectionE3;
	LocalEvent testLocal1;//局部事件1

	//注册局部事件1
	connectionE3 = RegEvent(&testLocal1, 2, CTestEvent::GetInstance(), &CTestEvent::TestEvetn);

	EventConnection connectionE4;
	LocalEvent testLocal2;//局部事件2

	//注册局部事件2
	connectionE4 = RegEvent(&testLocal2, 2, CTestEvent::GetInstance(), &CTestEvent::TestEvetn2);

	TestArgs tArgs2(1);

	//触发局部事件1
	testLocal1.FireEvent(2, tArgs2);//执行CTestEvent::TestEvetn

	TestArgs tArgs3(1);

	//触发局部事件2
	testLocal2.FireEvent(2, tArgs3);//执行CTestEvent::TestEvetn2

	//销毁局部事件1
	connectionE3.SetNull();

	TestArgs tArgs4(1);
	testLocal1.FireEvent(2, tArgs4);//没有执行任何，无输出

	//销毁局部事件2
	connectionE4.SetNull();

	TestArgs tArgs5(1);
	testLocal2.FireEvent(2, tArgs5);//没有执行任何，无输出

 */

//注册局部事件，用于继承了EventSet的类，events参数就是具体的类实例，eventype为事件id，与msgid一样，obj为接收的类对象，fun可以为类成员函数（事件回调）
//只会响应events里面的事件触发
#ifndef RegEvent
#define RegEvent(events, eventType, obj, fun)	(events)->SubscribeEvent(eventType, SubscriberSlot(fun, obj), __FILE__, __LINE__, __FUNCTION__)
#endif

//注册全局事件,eventype为事件id，与msgid一样，obj为接收的类对象，fun可以为类成员函数（事件回调）
//注册之后，与之对应的任意地方的eventtype触发，都会回调
#ifndef RegGlobalEventPtr
#define RegGlobalEventPtr(eventType, obj, fun)	GlobalEvent::GetInstance()->SubscribeEvent(eventType, SubscriberSlot(fun, obj), __FILE__, __LINE__, __FUNCTION__)
#endif

//触发全局事件
#ifndef FireGlobalEvent
#define FireGlobalEvent(eventType, args)	GlobalEvent::GetInstance()->FireEvent(eventType, args)
#endif

//事件注册返回接收
//可以用于销毁事件
struct EventConnection
{
	EventConnection()
	{

	}

	~EventConnection()
	{
		SetNull();
	}

	//销毁
	inline void SetNull()
	{
		if ((bool)con)
		{
			con->Disconnect();
			con = 0;
		}
	}

	inline void operator=(const Smart_Ptr<BoundSlot>& event_con)
	{
		con = event_con;
	}

	//是否是有效的
	inline bool IsValid() const
	{
		return (bool)con;
	}

private:
	Smart_Ptr<BoundSlot> con;
};

#endif /* EVENTDEFINE_H_ */
