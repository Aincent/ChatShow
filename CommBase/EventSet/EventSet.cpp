/*
 * EventSet.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#include "EventSet.h"
#include "BoundSlot.h"
#include "EventArgs.h"
#include "SubscriberSlot.h"
#include "../Memory/MemAllocator.h"


namespace CommBaseOut
{



EventSet::EventSet() : m_muted(false)
{

}

EventSet::~EventSet(void)
{
	RemoveAllEvents();
}

void EventSet::AddEvent(const DWORD& name)
{
	if (IsEventPresent(name))
	{
		return ;
	}

	m_events[name] = NEW CEEvent(name);
}

void EventSet::RemoveEvent(const DWORD& name)
{
	map<DWORD, CEEvent*>::iterator pos = m_events.find(name);

	if (pos != m_events.end())
	{
		delete pos->second;
		m_events.erase(pos);
	}
}

void EventSet::RemoveAllEvents(void)
{
	map<DWORD, CEEvent*>::iterator pos = m_events.begin();
	map<DWORD, CEEvent*>::iterator end = m_events.end()	;

	for (; pos != end; ++pos)
	{
		delete pos->second;
	}

	m_events.clear();
}

bool EventSet::IsEventPresent(const DWORD& name)
{
	return (m_events.find(name) != m_events.end());
}

Smart_Ptr<BoundSlot> EventSet::SubscribeEvent(const DWORD& name, SubscriberSlot subscriber, const char* pFileName, size_t line, const char* pFunName)
{
	return GetEventObject(name, true)->Subscribe(subscriber, pFileName, line, pFunName);
}

Smart_Ptr<BoundSlot> EventSet::SubscribeEvent(const DWORD& name, CEEvent::Group group, SubscriberSlot subscriber, const char* pFileName, size_t line, const char* pFunName)
{
	return GetEventObject(name, true)->Subscribe(group, subscriber, pFileName, line, pFunName);
}

void EventSet::FireEvent(const DWORD& name, EventArgs& args)
{
	FireEvent_impl(name, args);
}

bool EventSet::IsMuted(void) const
{
	return m_muted;
}

void EventSet::SetMutedState(bool setting)
{
	m_muted = setting;
}

CEEvent* EventSet::GetEventObject(const DWORD& name, bool autoAdd)
{
	map<DWORD, CEEvent*>::iterator pos = m_events.find(name);
	if (pos == m_events.end())
	{
		if (autoAdd)
		{
			AddEvent(name);
			return m_events.find(name)->second;
		}
		else
		{
			return 0;
		}
	}

	return pos->second;
}

void EventSet::FireEvent_impl(const DWORD& name, EventArgs& args)
{
	CEEvent* ev = GetEventObject(name);

	if ((ev != 0) && !m_muted)
		(*ev)(args);
}

}
