/*
 * CEEvent.cpp
 *
 *  Created on: Jun 10, 2017
 *      Author: root
 */
#include "CEEvent.h"
#include "../Log/Logger.h"
#include <algorithm>
#include "SubscriberSlot.h"
#include "EventArgs.h"

namespace CommBaseOut
{

class SubComp
{
public:
	SubComp(const BoundSlot& s) : d_s(s)
	{

	}

	bool operator()(std::pair<CEEvent::Group, Smart_Ptr<BoundSlot> > e) const
	{
		return *(e.second) == d_s;
	}

private:
	void operator=(const SubComp&);
	const BoundSlot& d_s;
};

CEEvent::CEEvent(const DWORD& name) : m_name(name)
{

}

CEEvent::~CEEvent()
{
	SlotContainer::iterator iter(m_slots.begin());
	const SlotContainer::const_iterator end_iter(m_slots.end());

	for (; iter != end_iter; ++iter)
	{
		iter->second->m_event = 0;
		iter->second->m_subscriber->Cleanup();
	}

	m_slots.clear();
}

Smart_Ptr<BoundSlot> CEEvent::Subscribe(const Subscriber& slot, const char* pFileName, size_t line, const char* pFunName)
{
	return Subscribe(static_cast<Group>(-1), slot, pFileName, line, pFunName);
}

Smart_Ptr<BoundSlot> CEEvent::Subscribe(Group group, const Subscriber& slot, const char* pFileName, size_t line, const char* pFunName)
{
	Smart_Ptr<BoundSlot> c = NEW BoundSlot(group, slot, *this, pFileName, line, pFunName);

	m_slots.insert(std::pair<Group, Smart_Ptr<BoundSlot> >(group, c));

	return c;
}


void CEEvent::operator()(EventArgs& args)
{
	std::vector<Smart_Ptr<BoundSlot> > conList;
	{
		conList.reserve(m_slots.size());
		SlotContainer::iterator itr = m_slots.begin();
		for (; itr != m_slots.end(); ++itr)
			conList.push_back(itr->second);
	}

	for (size_t i = 0; i < conList.size(); ++i)
	{
		Smart_Ptr<BoundSlot>& con = conList[i];
		if (con && con->Connected() == true)
		{
			bool bFire = false;
			try
			{
				bFire = (*con->m_subscriber)(args);
			}
			catch(std::exception& e)
			{
				std::string logText("event std::exception");
				logText += e.what();
				con->GetDebufInfo(logText);

				LOG_BASE(FILEINFO, "%s", logText.c_str());
			}
			catch(...)
			{
				std::string logText("event ...");
				con->GetDebufInfo(logText);

				LOG_BASE(FILEINFO, "%s", logText.c_str());
			}

			if (bFire == true)
				++args.handled;
			else
			{
				args.bfire = true;
				break;
			}
		}
	}
}

void CEEvent::Unsubscribe(const BoundSlot& slot)
{
	SlotContainer::iterator curr = std::find_if(m_slots.begin(), m_slots.end(), SubComp(slot));
	if (curr != m_slots.end())
		m_slots.erase(curr);
}

}
