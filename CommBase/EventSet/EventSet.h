/*
 * EventSet.h
 *
 *  Created on: Jun 10, 2017
 *      Author: root
 */

#ifndef EVENTSET_EVENTSET_H_
#define EVENTSET_EVENTSET_H_

#include "../define.h"
#include "../Smart_Ptr.h"
#include <map>
#include "CEEvent.h"

using namespace std;

namespace CommBaseOut
{

class BoundSlot;
class EventArgs;
class SubscriberSlot;

class EventSet
{
public:
	// 构造
	EventSet();

	// 析造
	virtual ~EventSet(void);

	// 添加事件
	void AddEvent(const DWORD& name);

	// 移除事件
	void RemoveEvent(const DWORD& name);

	// 移除所有事件
	void RemoveAllEvents(void);

	// 判断事件是否存在
	bool IsEventPresent(const DWORD& name);

	// 注册事件
	virtual Smart_Ptr<BoundSlot> SubscribeEvent(const DWORD& name, SubscriberSlot subscriber, const char* pFileName, size_t line, const char* pFunName);
	virtual Smart_Ptr<BoundSlot> SubscribeEvent(const DWORD& name, CEEvent::Group group, SubscriberSlot subscriber, const char* pFileName, size_t line, const char* pFunName);

	// 分发事件
	virtual void FireEvent(const DWORD& name, EventArgs& args);

	// 当前是否允许分发事件
	bool IsMuted(void) const;

	// 设置是否允许分发
	void SetMutedState(bool setting);

protected:
	CEEvent* GetEventObject(const DWORD& name, bool autoAdd = false);
	void FireEvent_impl(const DWORD& name, EventArgs& args);

	map<DWORD, CEEvent*> m_events;
	bool	 m_muted;

private:
	EventSet(EventSet&);
	EventSet& operator=(EventSet&);
};

}

#endif /* EVENTSET_EVENTSET_H_ */
