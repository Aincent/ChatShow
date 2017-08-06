/*
 * FriendMgr.cpp
 *
 *  Created on: Jul 22, 2017
 *      Author: root
 */
#include "FriendMgr.h"
#include "Log/Logger.h"
#include "../Object/Player/Player.h"
#include "../MessageBuild.h"
#include "MessageStruct/ServerReturn2Int.pb.h"
#include "MessageStruct/ServerReturn3Int.pb.h"
#include "MessageCommonRet.h"
#include "util.h"
#include "SvrConfig.h"

FriendMgr::FriendMgr(Player * player) : m_ReceiveCounts(0) , m_SendCounts(0)
{
	m_player = player;
}

FriendMgr::~FriendMgr()
{

}

void FriendMgr::SetFriendsInfo(PlayerInfo::FriendInfoList &friendInfoList)
{
	for(int i = 0; i < friendInfoList.friends_size(); i++)
	{
		PlayerInfo::FriendInfo *friendInfo = friendInfoList.mutable_friends(i);
		FriendStruct friendStruct;
		friendStruct.charid = friendInfo->charid();

		//先去服组，之前数据库保存的是有服组的ID
		friendStruct.charid = GET_PLAYER_CHARID(friendStruct.charid);

		//数据库中存储去服组ID，防止合服导致该值改变
//		friendStruct.charid = CREATE_CHARID_GS(ServerConHandler::GetInstance()->GetServerID(),friendStruct.charid);

		friendStruct.friendname = friendInfo->friendname();
		friendStruct.sex 		= friendInfo->sex();

		switch(friendInfo->type())
		{
			case eGoodsFriends:
			{
				if((short)m_goodFriends.size() >= 100)
					break;

				m_goodFriends.push_back(friendStruct);
				break;
			}
			case eBlackFriends:
			{
				m_blackFriends.push_back(friendStruct);
				break;
			}
			default :
			{
				LOG_ERROR(FILEINFO, "friends type is error");
			}
		}
	}

	m_ReceiveCounts = friendInfoList.m_receivecounts();
	m_SendCounts    = friendInfoList.m_sendcounts();

	m_player->SetDataFlag(eFriendInfo);
	return ;
}

void FriendMgr::GetFriendsInfo(PlayerInfo::FriendInfoList *friendInfoList)
{
	vector<FriendStruct>::iterator it = m_goodFriends.begin();
	for(; it!=m_goodFriends.end(); ++it)
	{
		PlayerInfo::FriendInfo * friendInfo = friendInfoList->add_friends();
		friendInfo->set_type(eGoodsFriends);
		friendInfo->set_charid(GET_PLAYER_CHARID(it->charid));
		friendInfo->set_friendname(it->friendname.c_str());
		friendInfo->set_sex(it->sex);
	}

	it = m_blackFriends.begin();
	for(; it!=m_blackFriends.end(); ++it)
	{
		PlayerInfo::FriendInfo * friendInfo = friendInfoList->add_friends();
		friendInfo->set_type(eBlackFriends);
		friendInfo->set_charid(GET_PLAYER_CHARID(it->charid));
		friendInfo->set_friendname(it->friendname.c_str());
		friendInfo->set_sex(it->sex);
	}

	friendInfoList->set_m_receivecounts(m_ReceiveCounts);
	friendInfoList->set_m_sendcounts(m_SendCounts);

	return ;
}

//更新好友属性
void FriendMgr::UpdateFriendAttr(int64 charid, int attrType, int value)
{
	bool isFind = false;
	FriendStruct* pRef = NULL;
	for(size_t i=0; i<m_goodFriends.size(); ++i)
	{
		if(m_goodFriends[i].charid == charid)
		{
			pRef = &m_goodFriends[i];
			isFind = true;
			break;
		}
	}

	for(size_t i=0; i<m_blackFriends.size()&& !isFind; ++i)
	{
		if(m_blackFriends[i].charid == charid)
		{
			pRef = &m_blackFriends[i];
			break;
		}
	}

	if(pRef == NULL)
	{
		return;
	}

//	switch(attrType)
//	{
//	case eCharLv:
//	{
//		pRef->lv = value;
//		break;
//	}
//
//	case eFightPower:
//	{
//		pRef->m_Fighting = value;
//		break;
//	}
//
//	case eVIPLv:
//	{
//		pRef->m_VipLevel = value;
//		break;
//	}
//
//	default:
//		return;
//	}

	m_player->SetDataFlag(eFriendInfo);
}
