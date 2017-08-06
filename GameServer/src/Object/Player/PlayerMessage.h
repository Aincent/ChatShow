/*
 * PlayerMessage.h
 *
 *  Created on: Jun 28, 2017
 *      Author: root
 */

#ifndef PLAYERMESSAGE_H_
#define PLAYERMESSAGE_H_

#include "Network/MessageHandler.h"
#include "MsgDefineMacro.h"
#include "ServerMsgDefine.h"
#include "Timer/TimerInterface.h"
#include "EventSet/EventSet.h"
#include "Player.h"

using namespace std;
using namespace CommBaseOut;

//定义一个玩家缓存
struct PLayerCache
{
	bool bFlag;
	DWORD64 time;     //记录时间
	Smart_Ptr<Player>  playerCache;

	PLayerCache():bFlag(false),time(0),playerCache(NULL)
	{

	}
};

class Player;

class PlayerMessage : public Message_Handler,public Request_Handler,public Ack_Handler
{
public:
	virtual ~PlayerMessage();

	static PlayerMessage * GetInstance()
	{
		if(m_instance == 0)
		{
			m_instance = new PlayerMessage();
		}

		return m_instance;
	}

	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_GT2GM_CLIENTIN);
	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_GT2GM_PLAYERINFO);
	DEF_MSG_ACK_DECLARE_FUN_H(MSG_REQ_GM2DB_PLAYERINFO);
	DEF_MSG_ACK_DECLARE_FUN_H(MSG_REQ_GS2DB_SAVE_PLAYERDATA);
	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_GT2GM_PLAYEREXIT);
	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_GT2GM_SYN_ATTR);
	virtual void Handle_Message(Safe_Smart_Ptr<Message> &message);
	virtual void Handle_Request(Safe_Smart_Ptr<Message> &message);
	virtual void Handle_Ack(Safe_Smart_Ptr<Message> &message);

	void addSendDataCach(int64 charID, SaveDataInfo& sendData);
	void dealWithSaveResult(SaveDataInfo& data);

	const map<int64, Smart_Ptr<Player> >& getAllPlayer(){return m_allPlayer;}
	map<int64, Smart_Ptr<Player> >& getAllPlayerRef(){return m_allPlayer;}

	inline void GetPlayerByCharid(const int64 &id, Smart_Ptr<Player> &player)
	{
		map<int64, Smart_Ptr<Player> >::iterator it = m_allPlayer.find(id);
		if(it != m_allPlayer.end())
		{
			player = it->second;
		}
	}

	inline void GetPlayerCacheByCharid(const int64 &id, Smart_Ptr<Player> &player)
	{
		map<int64, PLayerCache >::iterator it = m_PlayerCache.find(id);
		if(it != m_PlayerCache.end())
		{
			player = it->second.playerCache;
		}
	}

	void GetPlayerByKey(int key, Smart_Ptr<Player> &player)
	{
		map<int64, Smart_Ptr<Player> >::iterator it = m_allPlayer.begin();
		for(;it!=m_allPlayer.end();++it)
		{
			if(it->second->GetKey() == key)
			{
				player = it->second;
				break;
			}
		}
	}

	//玩家是否在线
	bool IsOnline(int64 charid);
	//玩家是否在线（判断真正在场景中）
	bool IsOnlineEx(int64 charid);

	//定时从玩家缓存中移除延时玩家
	void RemoveDelayPlayer(void* arg);

	bool PlayerLogout(Smart_Ptr<Player> pObj);

	int64 GetCharidByName(string name);

	void DestroyInstance()
	{
		if(m_instance)
		{
			delete m_instance;
			m_instance = 0;
		}
	}

private:
	PlayerMessage();

private:
	static PlayerMessage * m_instance;

	TimerConnection m_timerID;

	map<int64, Smart_Ptr<Player> > m_allPlayer;
	map<int64, PLayerCache > m_PlayerCache;
	std::map<int64,SaveDataInfo> m_saveDataList;//玩家信息保存临时缓存数据SaveDataInfo& sendData
};

#endif /* PLAYERMESSAGE_H_ */
