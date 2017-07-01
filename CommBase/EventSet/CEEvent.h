/*
 * CEEvent.h
 *
 *  Created on: Jun 10, 2017
 *      Author: root
 */

#ifndef EVENTSET_CEEVENT_H_
#define EVENTSET_CEEVENT_H_

#include "../Smart_Ptr.h"
#include "../define.h"
#include "BoundSlot.h"
#include "../Memory/MemAllocator.h"


namespace CommBaseOut
{

class SubscriberSlot;
class EventArgs;

class CEEvent
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:

	typedef SubscriberSlot Subscriber;

	typedef unsigned int Group;

	CEEvent(const DWORD& name);

	 ~CEEvent();

	// 事件名
	inline const DWORD& GetName(void) const
	{
		return m_name;
	}

	// 注册响应函数
	Smart_Ptr<BoundSlot> Subscribe(const Subscriber& slot, const char* pFileName, size_t line, const char* pFunName);
	Smart_Ptr<BoundSlot> Subscribe(Group group, const Subscriber& slot, const char* pFileName, size_t line, const char* pFunName);

	// 分发事件
	void operator()(EventArgs& args);

private:
	friend void BoundSlot::Disconnect();

	// 解除注册
	void Unsubscribe(const BoundSlot& slot);

	typedef std::multimap<Group, Smart_Ptr<BoundSlot> > SlotContainer;
	SlotContainer m_slots;
	const DWORD m_name;

private:
	CEEvent(const CEEvent&);
	CEEvent& operator=(const CEEvent&);
};
}


#endif /* EVENTSET_CEEVENT_H_ */
