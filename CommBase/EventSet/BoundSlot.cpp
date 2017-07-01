/*
 * BoundSlot.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "BoundSlot.h"
#include "SubscriberSlot.h"
#include "CEEvent.h"

namespace CommBaseOut
{
BoundSlot::BoundSlot(Group group, const SubscriberSlot& subscriber, CEEvent& event, const char* pFileName, size_t line, const char* pFunName)
	: m_group(group),
	m_subscriber(NEW SubscriberSlot(subscriber)),
	m_event(&event),
	m_fileName(pFileName),
	m_lineSize(line),
	m_funName(pFunName)
{
	if (m_fileName.size() == 0)
		m_fileName = "null";

	if (m_funName.size() == 0)
		m_funName = "null";
}


BoundSlot::BoundSlot(const BoundSlot& other)
	: m_group(other.m_group),
	m_subscriber(other.m_subscriber),
	m_event(other.m_event),
	m_fileName(other.m_fileName),
	m_lineSize(other.m_lineSize),
	m_funName(other.m_funName)
{

}


BoundSlot::~BoundSlot()
{
	Disconnect();
	delete m_subscriber;
}

BoundSlot& BoundSlot::operator=(const BoundSlot& other)
{
	m_group      = other.m_group;
	m_subscriber = other.m_subscriber;
	m_event      = other.m_event;
	m_fileName   = other.m_fileName;
	m_lineSize   = other.m_lineSize;
	m_funName    = other.m_funName;

	return *this;
}


bool BoundSlot::operator==(const BoundSlot& other) const
{
	return m_subscriber == other.m_subscriber;
}


bool BoundSlot::operator!=(const BoundSlot& other) const
{
	return !(*this == other);
}


bool BoundSlot::Connected() const
{
	return (m_subscriber != 0) && m_subscriber->Connected();
}


void BoundSlot::Disconnect()
{
	if (Connected())
		m_subscriber->Cleanup();

	if (m_event)
	{
		m_event->Unsubscribe(*this);
		m_event = 0;
	}
}
}
