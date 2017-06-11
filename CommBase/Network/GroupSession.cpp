/*
 * GroupSession.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "GroupSession.h"
#include "../Log/Logger.h"

namespace CommBaseOut
{

GroupSession::GroupSession(Context * c):m_c(c)
{
}

GroupSession::~GroupSession()
{

}

void GroupSession::DeleteGroupSession(int group, int channel)
{
	GUARD_WRITE(CRWLock, obj, &m_groupLock);
	map<int, GroupSessionInfo>::iterator itCh = m_groupChannel.find(group);
	if(itCh == m_groupChannel.end())
		return;

	bool isDel = itCh->second.DeleteChannel(channel);
	if(!isDel)
	{
		map<WORD, int>::iterator it = m_group.begin();
		for(; it!=m_group.end(); ++it)
		{
			if(it->second == group)
			{
				m_group.erase(it);
				break;
			}
		}
	}
}

int GroupSession::AddGroupSession(int type, int id, int channel, int count, bool &isSuccess)
{
	WORD key = (type << 8) | id;
	int ret = -1;

	GUARD_WRITE(CRWLock, obj, &m_groupLock);
	map<WORD, int>::iterator it = m_group.find(key);
	if(it == m_group.end())
	{
		GroupSessionInfo info;

		info.channel.push_back(channel);
		info.count = count;
		m_group[key] = channel;
		ret = channel;
		m_groupChannel[channel] = info;

		if(info.channel.size() == (DWORD)count)
		{
			isSuccess = true;
		}

		ret = channel;
	}
	else
	{
		if(m_groupChannel[it->second].channel.size() >= m_groupChannel[it->second].count)
		{
			return -2;
		}
		else if(m_groupChannel[it->second].channel.size() == (DWORD)(m_groupChannel[it->second].count - 1))
		{
			isSuccess = true;
		}

		ret = it->second;
		m_groupChannel[it->second].channel.push_back(channel);
	}

	return ret;
}

void GroupSession::UnBindSession(int64 key, int group)
{
//	if( group < 0 || key < 0 )
//		return ;
//
//	GUARD_WRITE(CRWLock, obj1, &m_keyLock);
//	map<int, map<int64, int> >::iterator itKey = m_keyChannel.find(group);
//	if(m_keyChannel.end() == itKey)
//		return ;
//
//	map<int64, int>::iterator it = itKey->second.find(key);
//	if(it != itKey->second.end())
//	{
//		itKey->second.erase(it);
//	}
}

int GroupSession::BindSession(int64 key, int group)
{
	if( group < 0 || key < 0 )
		return group;

	int channel = -1;

	GUARD_WRITE(CRWLock, obj, &m_groupLock);
	map<int, GroupSessionInfo>::iterator itCh = m_groupChannel.find(group);
	if(itCh == m_groupChannel.end())
	{
		LOG_BASE(FILEINFO, "bind session error and group[%d] is null", group);
		return group;
	}

	if(itCh->second.channel.size() <= 0)
	{
		LOG_BASE(FILEINFO, "group[%d] channel size is null and bind session error", group);
		return group;
	}

	channel = itCh->second.GetChannel((int)key);
//	obj.UnLock();
//
//	GUARD_WRITE(CRWLock, obj1, &m_keyLock);
//	map<int, map<int64, int> >::iterator itKey = m_keyChannel.find(group);
//	if(itKey == m_keyChannel.end())
//	{
//		map<int64, int> tKey;
//
//		tKey[key] = channel;
//		m_keyChannel[group] = tKey;
//	}
//	else
//	{
//		itKey->second[key] = channel;
//	}

	return channel;
}
}

