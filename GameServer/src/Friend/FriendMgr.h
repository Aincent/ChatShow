/*
 * FriendMgr.h
 *
 *  Created on: Jul 22, 2017
 *      Author: root
 */

#ifndef FRIENDMGR_H_
#define FRIENDMGR_H_

#include <vector>
#include <set>
#include "Smart_Ptr.h"
#include "MsgCommonClass.h"
#include "MessageStruct/CharLogin/PlayerInfo.pb.h"
#include "Timer/TimerInterface.h"
#include "MessageCommonRet.h"
#include "CharDefine.h"
#include "../EventSet/EventDefine.h"
using namespace std;
using namespace CommBaseOut;

enum FriendType
{
	eGoodsFriends = 1,
	eBlackFriends,
};

enum FriendOnlineType
{
	eOffline = 0,
	eOnline,
};


struct FriendStruct
{
	int     type;
	int64   charid;
	string  friendname;
	BYTE    sex;
	int 	point;

	FriendStruct():type(0),charid(0),sex(0),point(0)
	{
	}
};

class Player;

class FriendMgr
{
public:
	FriendMgr(Player * player);

	~FriendMgr();

	void SetFriendsInfo(PlayerInfo::FriendInfoList &friendInfoList);

	void GetFriendsInfo(PlayerInfo::FriendInfoList *friendInfoLis);

	void UpdateFriendAttr(int64 charid, int attrType, int value);
private:
	Player 				 *m_player;   			//玩家指针
	vector<FriendStruct> m_goodFriends;  		//好友容器
	vector<FriendStruct> m_blackFriends; 		//黑名单
	short	    		 m_ReceiveCounts;
	short				 m_SendCounts;
};

#endif /* FRIENDMGR_H_ */
