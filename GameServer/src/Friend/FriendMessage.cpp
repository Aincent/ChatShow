/*
 * FriendMessage.cpp
 *
 *  Created on: Jul 23, 2017
 *      Author: root
 */
#include "FriendMessage.h"
#include "Log/Logger.h"
#include "Network/Message_Facade.h"
#include "Network/NetWorkConfig.h"
#include "../MessageBuild.h"
#include "MsgCommonClass.h"
#include "MessageStruct/CharLogin/CharBaseInfo.pb.h"
#include "MessageStruct/ServerReturn2Int.pb.h"
#include "MessageStruct/ServerReturnInt.pb.h"
#include "MessageStruct/ServerReturn3Int.pb.h"
#include "../Dependence/MessageCommonRet.h"
#include "FriendMgr.h"
#include "../Object/Player/PlayerMessage.h"
#include "../Object/Player/Player.h"
#include "ServerEventDefine.h"

FriendMessage * FriendMessage::m_instance = 0;

FriendMessage::FriendMessage()
{

//	startClearTimer();
//
	m_OfflineEvent = RegGlobalEventPtr(PLAYER_OFFLINE,this,&FriendMessage::offline);

}

FriendMessage::~FriendMessage()
{
	if(m_ClearTimer.IsValid())
	{
		m_ClearTimer.SetNull();
	}

	if(m_OfflineEvent.IsValid())
	{
		m_OfflineEvent.SetNull();
	}
}

void FriendMessage::Handle_Message(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN


	DEF_SWITCH_TRY_DISPATCH_END
}

void FriendMessage::Handle_Request(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN



	DEF_SWITCH_TRY_DISPATCH_END
}

void FriendMessage::Handle_Ack(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

	DEF_SWITCH_TRY_DISPATCH_END
}

//param1: 触发事件的角色；  param2:响应事件的角色；  param3:属性类型；  param4:属性值
void FriendMessage::onEvent(int64 charID, int64 targetID, int attrType, int value)
{
	Smart_Ptr<Player> targetPlayer;
	PlayerMessage::GetInstance()->GetPlayerByCharid(targetID,targetPlayer);
	if(targetPlayer.Get()==NULL)
	{
		return;
	}

	targetPlayer->GetFriendManager()->UpdateFriendAttr(charID, attrType, value);
}

bool FriendMessage::offline(const EventArgs& e)
{
	const PlayerArgs& tempArgs = static_cast<const PlayerArgs& >(e);
	if(tempArgs.m_Player.Get()==NULL)
	{
		return true;
	}

	//清除好友关注对象
	int64 charid = tempArgs.m_Player->GetID();
	clearWatch( charid );

	return true;
}

void FriendMessage::clearWatch(int64 charID)
{
		std::map<int64,std::set<int64> >::iterator it = m_watch_list.find(charID);
		if(it == m_watch_list.end())
			return;

		std::set<int64>::iterator itChar = it->second.begin();
		for(; itChar != it->second.end(); ++itChar)
		{
			std::map<int64,std::set<int64> >::iterator it_gloable = m_event_list.find(*itChar);
			if(it_gloable != m_event_list.end())
			{
				it_gloable->second.erase(charID);
				if(it_gloable->second.empty())
					m_event_list.erase(it_gloable);
			}
		}

		//清除个人列表
		it->second.clear();
		m_watch_list.erase(it);
}

void FriendMessage::cancelWatch(int64 selfID, int64 targetID)
{
		std::map<int64,std::set<int64> >::iterator it = m_event_list.find(targetID);
		if(it==m_event_list.end()) return;

		it->second.erase(selfID);
		if(it->second.empty())
		{
			m_event_list.erase(it);
		}
}

void FriendMessage::watch(int64 selfID, int64 targetID)
{
		std::set<int64>	& refCharList =	m_event_list[targetID];
		std::set<int64>::iterator it = refCharList.find(selfID);
		if(it==refCharList.end())
		{
			refCharList.insert(selfID);
			std::set<int64>& myWatch =	m_watch_list[selfID];
			myWatch.insert(targetID);
		}
}

void FriendMessage::dispatchEvent(int64 charID, int attrType, int value)
{
		std::map<int64,std::set<int64> >::iterator it = m_event_list.find(charID);
		if(it==m_event_list.end() || it->second.size()==0)
			return;

		std::set<int64>::iterator itChar = it->second.begin();
		for(; itChar != it->second.end(); ++itChar)
		{
			onEvent( charID, *itChar, attrType, value );
		}
}

int FriendMessage::GetFriendWatchSelf(int64 charID, set<int64>& watchSelf)
{
	std::map<int64,std::set<int64> >::iterator it = m_event_list.find(charID);
	if(it==m_event_list.end() || it->second.size()==0)
		return -1;

	watchSelf = it->second;

	return 0;
}
