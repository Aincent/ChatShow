/*
 * PlayerMessage.cpp
 *
 *  Created on: Jun 28, 2017
 *      Author: root
 */
#include "PlayerMessage.h"
#include "MessageStruct/ServerReturn2Int.pb.h"
#include "MessageStruct/ServerReturn3Int.pb.h"
#include "MessageStruct/ServerReturn4Int.pb.h"
#include "MessageStruct/ServerReturn5Int.pb.h"
#include "MessageStruct/ServerReturnInt.pb.h"
#include "MessageStruct/ServerReturnIntChar.pb.h"
#include "SvrConfig.h"
#include "Log/Logger.h"
#include "Network/Message_Facade.h"
#include "../../MessageBuild.h"
#include "../../ServerManager.h"
#include "Common.h"
#include "ServerMsgDefine.h"
#include "MsgCommonClass.h"
#include "MessageStruct/CharLogin/PlayerInfo.pb.h"
#include "MessageStruct/CharLogin/CharBaseInfo.pb.h"
#include "../ObjectFactory.h"

PlayerMessage * PlayerMessage::m_instance = 0;

PlayerMessage::PlayerMessage()
{
	m_PlayerCache.clear();
	//注册定时器
	if(m_timerID.IsValid())
	{
		m_timerID.SetNull();
	}

	DEF_MSG_REQUEST_REG_FUN(eGameServer, MSG_REQ_GT2GM_CLIENTIN);
	DEF_MSG_REQUEST_REG_FUN(eGameServer, MSG_REQ_GT2GM_PLAYERINFO);
	DEF_MSG_REQUEST_REG_FUN(eGameServer, MSG_REQ_GT2GM_PLAYEREXIT);
	DEF_MSG_ACK_REG_FUN(eGameServer, MSG_REQ_GM2DB_PLAYERINFO);
	DEF_MSG_ACK_REG_FUN(eGameServer, MSG_REQ_GS2DB_SAVE_PLAYERDATA);
	DEF_MSG_REQUEST_REG_FUN(eGameServer, MSG_REQ_GT2GM_SYN_ATTR);

	m_timerID = TimerInterface::GetInstance()->RegRepeatedTime(&PlayerMessage::RemoveDelayPlayer, this, 0, 0, 0, 0,20 * 1000, 20 * 1000);
}


PlayerMessage::~PlayerMessage()
{
	if(m_timerID.IsValid())
	{
		m_timerID.SetNull();
	}

	Smart_Ptr<Player> nullPlayer;
	map<int64, Smart_Ptr<Player> >::iterator it = m_allPlayer.begin();
	for (; it != m_allPlayer.end(); ++it)
	{
		it->second->SetMyself(nullPlayer);
		cout << endl;
		cout << "~PlayerMessage  SetMyself  Charid "
		                << it->second->GetID() << " playerdd "
		                << &it->second << endl;
		cout << endl;
	}

	map<int64, PLayerCache >::iterator itCa = m_PlayerCache.begin();
	for (; itCa != m_PlayerCache.end(); ++itCa)
	{
		itCa->second.playerCache->SetMyself(nullPlayer);
		cout << endl;
		cout << "~PlayerMessage  SetMyself  Charid "
		                << itCa->second.playerCache->GetID()
		                << " playerdd " << &itCa->second.playerCache
		                << endl;
		cout << endl;
	}

	m_PlayerCache.clear();
	m_allPlayer.clear();
}

void PlayerMessage::Handle_Message(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

	DEF_SWITCH_TRY_DISPATCH_END
}

void PlayerMessage::Handle_Request(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_GT2GM_CLIENTIN);
	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_GT2GM_PLAYERINFO);
	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_GT2GM_PLAYEREXIT);
	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_GT2GM_SYN_ATTR);

	DEF_SWITCH_TRY_DISPATCH_END
}

void PlayerMessage::Handle_Ack(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN
//		DEF_MSG_ACK_DISPATCH_FUN(MSG_REQ_GT2WS_CLIENTIN);
		DEF_MSG_ACK_DISPATCH_FUN(MSG_REQ_GM2DB_PLAYERINFO);
		DEF_MSG_ACK_DISPATCH_FUN(MSG_REQ_GS2DB_SAVE_PLAYERDATA);
//		DEF_MSG_ACK_DISPATCH_FUN(MSG_REQ_GM2GT_PLAYER_ONLINE);

	DEF_SWITCH_TRY_DISPATCH_END
}

void PlayerMessage::addSendDataCach(int64 charID, SaveDataInfo& sendData)
{
	std::map<int64,SaveDataInfo>::iterator it=m_saveDataList.find(charID);
	if(it==m_saveDataList.end())
	{
		m_saveDataList[charID] = sendData;
	}
}

void PlayerMessage::dealWithSaveResult(SaveDataInfo& data)
{
	//次数达到之后才会处理
	data.count -= 1;
	if(data.count > 0 )
		return;

	if(data.content.Get() == NULL)
		return;

	//
	switch(data.msgtype)
	{
	case SimpleMessage:
	{
		break;
	}
	case Request:
	{
		Safe_Smart_Ptr<CommBaseOut::Message> messRet = build_message(data.msgid, data.message, data.content.Get(), Request);
		messRet->SetAct(data.m_act);
		Message_Facade::Send(messRet);
		break;
	}
	case Ack:
	{
		Safe_Smart_Ptr<CommBaseOut::Message> messRet = build_message(data.msgid, data.message, data.content.Get());
		Message_Facade::Send(messRet);
		break;
	}
	default:
		break;
	}
}

int64 PlayerMessage::GetCharidByName(string name)
{
	map<int64, Smart_Ptr<Player> >::iterator it;
	for (it = m_allPlayer.begin(); it != m_allPlayer.end(); ++it)
	{
		if (it->second->GetName() == name)
			return it->first;
	}
	return 0;
}

bool PlayerMessage::IsOnline(int64 charid)
{
	map<int64, PLayerCache >::iterator itCache = m_PlayerCache.find(charid);
	if (itCache != m_PlayerCache.end())
	{
		return true;
	}

	map<int64, Smart_Ptr<Player> >::iterator it = m_allPlayer.find(charid);
	if (it != m_allPlayer.end())
	{
		return true;
	}

	return false;
}

bool PlayerMessage::IsOnlineEx(int64 charid)
{
	map<int64, Smart_Ptr<Player> >::iterator it = m_allPlayer.find(charid);
	if (it != m_allPlayer.end())
	{
		return true;
	}

	return false;
}

void PlayerMessage::RemoveDelayPlayer(void* arg)
{
	DWORD64 ntime = CUtil::GetNowSecond();
	DWORD64 ndeteTime = 0;

	map<int64, PLayerCache >::iterator ite = m_PlayerCache.begin();
	for (; ite != m_PlayerCache.end();)
	{
		if(ite->second.bFlag)
		{
			ndeteTime = 60 * 1000;
		}
		else
		{
			ndeteTime = 15 * 1000;
		}

		DWORD64 deftime = ntime - ite->second.time;
		if (deftime > ndeteTime)
		{
			Smart_Ptr<Player> player = ite->second.playerCache;

			player->SetOffLineTime(ntime);

			LOG_ERROR(FILEINFO, "remove delay nowtime[%lld], oldtime[%lld], player[%lld] and gameserver request gt exit", ntime, ite->second.time, GET_PLAYER_CHARID(player->GetID()));
			if(player->GetType() == ePlayer)
			{
				//退出除副本以外的信息
//				PlayerRemoveMemberEventArgs tArgs(player,GROUP_REMOVEMETHOD_OUT);
//				FireGlobalEvent(PlAYER_LOGINOUT_CLEAR_EVENT_TYPE, tArgs);
//				player->FireEvent(PlAYER_LOGINOUT_CLEAR_EVENT_TYPE, tArgs);
				SaveDataInfo sendData;
				ite->second.playerCache->SaveAllPlayer(sendData,true,true);

				//通知GT玩家下线
				ServerReturn::ServerRetInt sim;
				sim.set_ret(player->GetID());

				Safe_Smart_Ptr<CommBaseOut::Message> messRet = build_message(MSG_SIM_GM2GT_PLAYER_EXIT, &sim, player->GetChannelID(), SimpleMessage);
				Message_Facade::Send(messRet);

				Smart_Ptr<Player> myself;

				ite->second.playerCache->SetMyself(myself);
				m_PlayerCache.erase(ite++);
			}
			else
			{
				Smart_Ptr<Player> myself;

				ite->second.playerCache->SetMyself(myself);
				m_PlayerCache.erase(ite++);
			}
		}
		else
		{
			++ite;
		}
	}
}

bool PlayerMessage::PlayerLogout(Smart_Ptr<Player> pObj)
{
	if(pObj && pObj->GetType() == ePlayer)
	{

	}

	return true;
}

//网关服请求游戏服玩家信息
DEF_MSG_REQUEST_DEFINE_FUN(PlayerMessage, MSG_REQ_GT2GM_PLAYERINFO)
{
	ServerReturn::ServerRetIntChar req;
	ServerReturn::ServerRetInt con;
	int len = 0;
	char *str = message->GetBuffer(len);

	req.ParseFromArray(str, len);

	LOG_DEBUG(FILEINFO, "Player login on and get playerinfo[%lld]", GET_SERVER_CHARID(req.retf()));

	Smart_Ptr<SvrItem> dbSvr = NULL;

	if(IsOnline(req.retf()))
	{
		LOG_ERROR(FILEINFO, "player[%lld] is online", GET_SERVER_CHARID(req.retf()));
		return;
	}

	//找到DB服
	ServerConHandler::GetInstance()->GetDBServerBySvrID(GET_SERVER_CHARID(req.retf()),dbSvr);
	if(!dbSvr)
	{
		LOG_FATAL(FILEINFO, "Gameserver find no DBServer in server [serverID=%lld]",GET_SERVER_CHARID(req.retf()));
		return;
	}

	con.set_ret(GET_PLAYER_CHARID(req.retf()));

	//建立游戏服向DB服请求玩家信息
	Safe_Smart_Ptr<CommBaseOut::Message> messRet = build_message(MSG_REQ_GM2DB_PLAYERINFO, &con, dbSvr.Get(), Request);
	string client_ip = req.rets();

	messRet->SetAct(new StringIntAct(message, 0, 0, client_ip));
	Message_Facade::Send(messRet);
}

//游戏服请求DB服玩家信息
DEF_MSG_ACK_DEFINE_FUN(PlayerMessage, MSG_REQ_GM2DB_PLAYERINFO)
{
	//请求超时
	if (message->GetErrno() == eReqTimeOut)
	{
		LOG_DEBUG(FILEINFO, "gameserver request dbserver player info and ack timeout");
		return;
	}

	//声明玩家信息的各个结构体
	PlayerInfo::PlayerInfo content;
	int len = 0;
	int64 nowTime = CUtil::GetNowSecond();

	char *con = message->GetBuffer(len);
	content.ParseFromArray(con, len);
	const PlayerInfo::BaseInfo& bsinfo = content.bsinfo();

	if(IsOnline(bsinfo.charid()))
	{
		LOG_ERROR(FILEINFO, "player[%lld] is online", bsinfo.charid());
		return;
	}

	Smart_Ptr<Object> obj = ObjectFactory::GetInstance()->AllocObject(ePlayer);

	//将obj定为一个玩家Player
	Smart_Ptr<Player> player = obj;

	player->SetMyself(player);

	//设置玩家登录时间
	player->SetLoginTime(nowTime);

	player->SetChannelID(static_cast<StringIntAct *>(act.Get())->mesReq->GetChannelID());

	player->SetDBChannelID(Message_Facade::BindGroupChannel(static_cast<StringIntAct *>(act.Get())->mesReq->GetChannelID(), ServerConHandler::GetInstance()->GetDBChannelBySvrID(GET_SERVER_CHARID(player->GetID()))));

	player->InitBaseInfo(content.bsinfo());

	PLayerCache cache;
	cache.playerCache = player;
	cache.time = nowTime;
	cache.bFlag = true;

	m_PlayerCache[bsinfo.charid()] = cache;

	if(!(static_cast<StringIntAct *>(act.Get())->mesDataf))
	{		//刚登录的
		player->SetIP(static_cast<StringIntAct *>(act.Get())->mesDatafo);  //刚登录设置 玩家的 IP信息
	}

#ifdef DEBUG
	int endTime = CUtil::GetNowSecond();

	if(endTime - nowTime > 100)
	{
		printf("\n 玩家初始化时间： time=%lld \n", endTime - nowTime );
		LOG_FATAL(FILEINFO, "ack player info and need more time[%lld]", endTime - nowTime);
	}
#endif

	if(!(static_cast<StringIntAct *>(act.Get())->mesReq))
	{
		LOG_DEBUG(FILEINFO, "player login in game and finish player[%lld] info ", GET_PLAYER_CHARID(player->GetID()));
		CharLogin::CharBaseInfo info;
		player->SetCharBaseInfo(&info);
		Safe_Smart_Ptr<CommBaseOut::Message> messRet = build_message(MSG_REQ_GT2GM_PLAYERINFO,static_cast<commonAct *>(act.Get())->mesReq,&info);
		messRet->SetMessageTime(info.charid());
		Message_Facade::Send(messRet);
	}
}

DEF_MSG_ACK_DEFINE_FUN(PlayerMessage, MSG_REQ_GS2DB_SAVE_PLAYERDATA)
{
	int64 charID = static_cast<IntAct*>(act.Get())->mesData;
	std::map<int64,SaveDataInfo>::iterator it=m_saveDataList.find(charID);
	if(it==m_saveDataList.end())
	{
		LOG_DEBUG(FILEINFO, "\n 请求保存玩家数据时返回找不到玩家：id=%lld \n", charID);
		return;
	}

	dealWithSaveResult(it->second);
	m_saveDataList.erase(it);
}


//网关服发消息通知clientin
DEF_MSG_REQUEST_DEFINE_FUN(PlayerMessage, MSG_REQ_GT2GM_CLIENTIN)
{
	ServerReturn::ServerRetInt meContent;
	char *con;
	int len = 0;
	con = message->GetBuffer(len);
	meContent.ParseFromArray(con, len);

	int64 charID = message->GetMessageTime();
	map<int64, Smart_Ptr<Player> >::iterator itMap = m_allPlayer.find(charID);
	if (itMap != m_allPlayer.end())
	{
		LOG_ERROR(FILEINFO, "          player[%lld] already online        ", charID);
		return;
	}

	map<int64, PLayerCache >::iterator it = m_PlayerCache.find(charID);
	if(it != m_PlayerCache.end())
	{
		Smart_Ptr<Player> tempPlayer = it->second.playerCache;
		Smart_Ptr<Object> obj = tempPlayer;

		//进入事件触发
		if(tempPlayer)
		{
//			PlayerLoginEventArgs tArgs(tempPlayer,GET_MAP_ID(obj->GetMapID()));
//			tempPlayer->FireEvent(PLAYER_ENTERMAP_EVENT_TYPE, tArgs);
		}

		//玩家登入完成，添加到玩家列表里，删除Cache值
		m_PlayerCache.erase(it);
		m_allPlayer[charID] = tempPlayer;

		Safe_Smart_Ptr<CommBaseOut::Message> gatemess = build_message(MSG_REQ_GT2GM_CLIENTIN, message, &meContent);
		Message_Facade::Send(gatemess);
	}
	else
	{
		LOG_ERROR(FILEINFO, "MSG_REQ_GT2GM_CLIENTIN: player[%lld] login in timeout, time=%lld", GET_PLAYER_CHARID(charID), CUtil::GetNowSecond());
	}
}

DEF_MSG_REQUEST_DEFINE_FUN(PlayerMessage, MSG_REQ_GT2GM_PLAYEREXIT)
{
	ServerReturn::ServerDoubleInt meContent;
	Smart_Ptr<ServerReturn::ServerRetInt> ack = new ServerReturn::ServerRetInt();
	char *con;
	int len = 0;

	con = message->GetBuffer(len);
	meContent.ParseFromArray(con, len);
	ack->set_ret(meContent.retf());

	int64 offlineTime = CUtil::GetNowSecond();
	//下线，清玩家缓存
	map<int64, PLayerCache >::iterator ite = m_PlayerCache.find(meContent.retf());
	if (ite != m_PlayerCache.end())
	{
		ite->second.playerCache->SetOffLineTime(offlineTime);
		if (ite->second.playerCache->GetType() == ePlayer)
		{
			PlayerLogout(ite->second.playerCache);
			SaveDataInfo sendData;
			sendData.msgtype = Ack;
			sendData.msgid = MSG_REQ_GT2GM_PLAYEREXIT;
			sendData.message = message;
			sendData.content = ack;
			ite->second.playerCache->SaveAllPlayer(sendData,true,true);

			Smart_Ptr<Player> myself;
			ite->second.playerCache->SetMyself(myself);
		}
		m_PlayerCache.erase(ite);
	}
	else
	{
		//从玩家列表中移出
		map<int64, Smart_Ptr<Player> >::iterator it = m_allPlayer.find(meContent.retf());
		if (it != m_allPlayer.end())
		{
			it->second->SetOffLineTime(offlineTime);
			Smart_Ptr<Object> obj = it->second;
			if (obj->GetType() == ePlayer)
			{
				PlayerLogout(it->second);
				{
					SaveDataInfo sendData;
					sendData.msgtype = Ack;
					sendData.msgid = MSG_REQ_GT2GM_PLAYEREXIT;
					sendData.message = message;
					sendData.content = ack;
					it->second->SaveAllPlayer(sendData,true,true);
				}
				Smart_Ptr<Player> myself;
				it->second->SetMyself(myself);
			}
			m_allPlayer.erase(it);
		}
		else
		{
			Safe_Smart_Ptr<CommBaseOut::Message> messRet = build_message(MSG_REQ_GT2GM_PLAYEREXIT, message, ack.Get());
			Message_Facade::Send(messRet);
		}
	}
}

//更新玩家属性
DEF_MSG_REQUEST_DEFINE_FUN(PlayerMessage, MSG_REQ_GT2GM_SYN_ATTR)
{
	CharLogin::SynCharBase meContent;
	char *con;
	int len = 0;

	con = message->GetBuffer(len);
	meContent.ParseFromArray(con, len);
	int64 charID = message->GetMessageTime();
	Smart_Ptr<Player> player;
	GetPlayerByCharid(charID, player);
	if(player)
	{
		player->SetTypeValue(meContent);
	}
}
