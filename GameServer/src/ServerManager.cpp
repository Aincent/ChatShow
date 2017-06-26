/*
 * ServerManager.cpp
 *
 *  Created on: Jun 27, 2017
 *      Author: root
 */
#include "ServerManager.h"
#include "./Log/Logger.h"
#include "util.h"
#include "./Network/Message_Facade.h"
#include "SvrConfig.h"
#include "MessageStruct/ServerReturnInt.pb.h"
#include "MessageBuild.h"

ServerConHandler *ServerConHandler::m_instance = 0;

ServerConHandler::ServerConHandler():m_localID(-1),m_localType(0),m_serverID(-1)
{

}

ServerConHandler::~ServerConHandler()
{
	m_gateSvr.clear();
	m_dbSvr.clear();
	m_allSvr.clear();
}

void ServerConHandler::on_new_channel_build(int channel_id,short int local_id,unsigned char local_type,short int remote_id,unsigned char remote_type,Safe_Smart_Ptr<Inet_Addr> remote_address)
{
	if(m_localID == -1)
	{
		m_localID = local_id;
		m_localType = local_type;
	}

	switch(remote_type)
	{
	case eGateServer:
		{
			GUARD(CSimLock, obj, &m_allLock);
			map<int,Smart_Ptr<SvrItem> >::iterator it = m_allSvr.find(channel_id);
			if(it != m_allSvr.end())
			{
				obj.UnLock();
				LOG_WARNING(FILEINFO,"GateServer new channel build again closed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

				Message_Facade::CloseChannel(channel_id);

				break;
			}

			LOG_INFO(FILEINFO,"GateServer new channel build [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

			Smart_Ptr<SvrItem> item = new SvrItem();

			item->channelID = channel_id;
			item->ip = remote_address->GetIPToChar();
			item->remoteID = remote_id;
			item->remoteType = remote_type;
			m_allSvr[channel_id] = item;
			obj.UnLock();

			break;
		}
	case eDBServer:
		{
//			GUARD(CSimLock, obj, &m_allLock);
			map<int,Smart_Ptr<SvrItem> >::iterator it = m_allSvr.find(channel_id);
			if(it != m_allSvr.end())
			{
//				obj.UnLock();
				LOG_WARNING(FILEINFO,"dbServer new channel build again closed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

				Message_Facade::CloseChannel(channel_id);

				break;
			}

			LOG_INFO(FILEINFO,"db new channel build [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

			Smart_Ptr<SvrItem> item = new SvrItem();

			item->channelID = channel_id;
			item->ip = remote_address->GetIPToChar();
			item->remoteID = remote_id;
			item->remoteType = remote_type;
			m_allSvr[channel_id] = item;

			break;
		}
	case ePathServer:
		{
			if((bool)m_pathSvr)
			{
				LOG_WARNING(FILEINFO,"PathServer new channel build again closed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

				Message_Facade::CloseChannel(channel_id);

				break;
			}

			m_pathSvr = new SvrItem();

			m_pathSvr->channelID = channel_id;
			m_pathSvr->ip = remote_address->GetIPToChar();
			m_pathSvr->remoteID = remote_id;
			m_pathSvr->remoteType = remote_type;

			LOG_INFO(FILEINFO,"pathserver new channel build [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

			break;
		}
	case eWorldServer:
		{
			map<int,Smart_Ptr<SvrItem> >::iterator it = m_allSvr.find(channel_id);
			if(it != m_allSvr.end())
			{
				LOG_WARNING(FILEINFO,"worldServer new channel build again closed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

				Message_Facade::CloseChannel(channel_id);

				break;
			}

			LOG_INFO(FILEINFO,"worldServer new channel build [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

			Smart_Ptr<SvrItem> item = new SvrItem();

			item->channelID = channel_id;
			item->ip = remote_address->GetIPToChar();
			item->remoteID = remote_id;
			item->remoteType = remote_type;
			m_allSvr[channel_id] = item;
			break;
		}
	case eManagerServer:
	{
		map<int,Smart_Ptr<SvrItem> >::iterator it = m_allSvr.find(channel_id);
		if(it != m_allSvr.end())
		{
			LOG_WARNING(FILEINFO,"ManagerServer new channel build again closed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());
			Message_Facade::CloseChannel(channel_id);
			break;
		}

		Smart_Ptr<SvrItem> item = new SvrItem();

		item->channelID = channel_id;
		item->ip = remote_address->GetIPToChar();
		item->remoteID = remote_id;
		item->remoteType = remote_type;
		m_allSvr[channel_id] = item;
		break;
	}
	default:
		{
			LOG_WARNING(FILEINFO,"unknown channel build [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());
			Message_Facade::CloseChannel(channel_id);

			break;
		}
	}
}

void ServerConHandler::on_channel_error(int channel_id,short int local_id,unsigned char local_type,short int remote_id,unsigned char remote_type,int error_code,Safe_Smart_Ptr<Inet_Addr> remote_address)
{
	switch(remote_type)
	{
	case eGateServer:
		{
//			PlayerMessage::GetInstance()->AllPlayerExitByChannel(channel_id);
			Smart_Ptr<SvrItem> item;
			map<int,Smart_Ptr<SvrItem> >::iterator it = m_allSvr.find(channel_id);
			if(it != m_allSvr.end())
			{
				item = it->second;
				m_allSvr.erase(it);
			}

			if(item)
			{
				map<int,Smart_Ptr<SvrItem> >::iterator itgt = m_gateSvr.find(item->serverID);
				if(itgt != m_gateSvr.end())
				{
					m_gateSvr.erase(itgt);
				}
			}

			Message_Facade::CloseChannel(channel_id);
			break;
		}
	case eDBServer:
		{
			Smart_Ptr<SvrItem> item;
			map<int,Smart_Ptr<SvrItem> >::iterator it = m_allSvr.find(channel_id);
			if(it != m_allSvr.end())
			{
				item = it->second;
				m_allSvr.erase(it);
			}

			if(item)
			{
				map<int,Smart_Ptr<SvrItem> >::iterator itgt = m_dbSvr.find(item->serverID);
				if(itgt != m_dbSvr.end())
				{
					m_dbSvr.erase(itgt);
				}
			}

			LOG_DEBUG(FILEINFO,"GameServer new channel build error [channelid = %d, remoteid=%d, remotetype=%d, ip=%s , errorno=%d]",channel_id, remote_id, remote_type, remote_address->GetIPToChar(), error_code);
			Message_Facade::CloseChannel(channel_id);
			break;
		}
	case ePathServer:
		{
			m_pathSvr->clear();
			LOG_DEBUG(FILEINFO,"PathServer new channel build error [channelid = %d, remoteid=%d, remotetype=%d, ip=%s , errorno=%d]",channel_id, remote_id, remote_type, remote_address->GetIPToChar(), error_code);
			Message_Facade::CloseChannel(channel_id);
			break;
		}
	case eManagerServer:
	{
		Smart_Ptr<SvrItem> item;
		map<int,Smart_Ptr<SvrItem> >::iterator it = m_allSvr.find(channel_id);
		if(it != m_allSvr.end())
		{
			item = it->second;
			m_allSvr.erase(it);
		}

		if(item)
		{
			map<int,Smart_Ptr<SvrItem> >::iterator itgt = m_managerSvr.find(item->serverID);
			if(itgt != m_managerSvr.end())
			{
				m_managerSvr.erase(itgt);
			}
		}

		LOG_DEBUG(FILEINFO,"ManagerServer new channel build error [channelid = %d, remoteid=%d, remotetype=%d, ip=%s , errorno=%d]",channel_id, remote_id, remote_type, remote_address->GetIPToChar(), error_code);
		Message_Facade::CloseChannel(channel_id);
		break;
	}
	default:
		{
			LOG_WARNING(FILEINFO,"unknown channel build error [channelid = %d, remoteid=%d, remotetype=%d, ip=%s , errorno=%d]",channel_id, remote_id, remote_type, remote_address->GetIPToChar(), error_code);
			Message_Facade::CloseChannel(channel_id);
			break;
		}
	}
}

void ServerConHandler::on_connect_failed(int connector_id,short int local_id,unsigned char local_type,short int remote_id,unsigned char remote_type,int error,Safe_Smart_Ptr<Inet_Addr> remote_address)
{
	switch(remote_type)
	{
	case eGateServer:
		{
			LOG_DEBUG(FILEINFO,"GateServer new channel build failed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",connector_id, remote_id, remote_type, remote_address->GetIPToChar());
			break;
		}
	case eDBServer:
		{
			LOG_DEBUG(FILEINFO,"DBServer new channel build failed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",connector_id, remote_id, remote_type, remote_address->GetIPToChar());

			break;
		}
	case ePathServer:
		{
			LOG_DEBUG(FILEINFO,"PathServer new channel build failed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",connector_id, remote_id, remote_type, remote_address->GetIPToChar());

			break;
		}
	case eManagerServer:
	{
		LOG_DEBUG(FILEINFO,"ManagerServer new channel build failed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",connector_id, remote_id, remote_type, remote_address->GetIPToChar());
		break;
	}
	default:
		{
			LOG_WARNING(FILEINFO,"unknown channel build failed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",connector_id, remote_id, remote_type, remote_address->GetIPToChar());
			break;
		}
	}
}

void ServerConHandler::GetDBServerBySvrID(short int id, Smart_Ptr<SvrItem> &dbSvr)
{
//	GUARD(CSimLock, obj, &m_dbLock);
	map<int,Smart_Ptr<SvrItem> >::iterator it = m_dbSvr.begin();
	for(; it!=m_dbSvr.end(); ++it)
	{
		if(id == it->first)
		{
			dbSvr = it->second;
		}
	}
}

int ServerConHandler::GetDBChannelBySvrID(int id)
{
//	GUARD(CSimLock, obj, &m_dbLock);
	map<int,Smart_Ptr<SvrItem> >::iterator it = m_dbSvr.begin();
	for(; it!=m_dbSvr.end(); ++it)
	{
		if(id == it->first)
		{
			return it->second->channelID;
		}
	}

	return -1;
}

void ServerConHandler::GetGateServerBySvrID(short int id, Smart_Ptr<SvrItem> &gtSvr)
{
//	GUARD(CSimLock, obj, &m_gateLock);
	map<int,Smart_Ptr<SvrItem> >::iterator it = m_gateSvr.begin();
	for(; it!=m_gateSvr.end(); ++it)
	{
		if(id == it->first)
		{
			gtSvr = it->second;
		}
	}
}
//得到一个随机GTserver（用于世界广播消息）
void ServerConHandler::GetGateServer(Smart_Ptr<SvrItem> &gtSvr)
{
//	GUARD(CSimLock, obj, &m_gateLock);
	map<int,Smart_Ptr<SvrItem> >::iterator it = m_gateSvr.begin();
	for(; it!=m_gateSvr.end(); ++it)
	{
		gtSvr = it->second;
		return;
	}
}

void ServerConHandler::BraodcastMessageToWorld(unsigned int msgType, unsigned int msgID, google::protobuf::Message *content)
{
	map<int,Smart_Ptr<SvrItem> >::iterator it = m_worldSvr.begin();
	for(; it!=m_worldSvr.end(); ++it)
	{
		Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(msgID, content, it->second->channelID, msgType);
		Message_Facade::Send(messRet);
	}
}

void ServerConHandler::Handle_Ack(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN
		DEF_MSG_ACK_DISPATCH_FUN(MSG_SIM_GT2GM_PUSHSERVERID);
		DEF_MSG_ACK_DISPATCH_FUN(MSG_SIM_DB2GM_PUSHSERVERID);
		DEF_MSG_ACK_DISPATCH_FUN(MSG_REQ_GM2WS_PUSHSERVERID);
		DEF_MSG_ACK_DISPATCH_FUN(MSG_REQ_GM2MS_PUSHSERVERID);
	DEF_SWITCH_TRY_DISPATCH_END
}

DEF_MSG_ACK_DEFINE_FUN(ServerConHandler, MSG_SIM_GT2GM_PUSHSERVERID)
{
	if(message->GetErrno() == eReqTimeOut)
	{
		LOG_WARNING(FILEINFO, "gameserver request gateserver push server id and ack timeout");

		return;
	}

	LOG_DEBUG(FILEINFO, "gateServer push server id");

	ServerReturn::ServerRetInt sim;
	int len = 0;
	char *content = message->GetBuffer(len);
	Smart_Ptr<SvrItem> item;

	sim.ParseFromArray(content, len);

//	GUARD(CSimLock, obj, &m_allLock);
	map<int,Smart_Ptr<SvrItem> >::iterator it = m_allSvr.find(message->GetChannelID());
	if(it != m_allSvr.end())
	{
		it->second->serverID = sim.ret();
		item = it->second;
	}
//	obj.UnLock();

//	GUARD(CSimLock, objgt, &m_gateLock);
	m_gateSvr[sim.ret()] = item;
//	objgt.UnLock();
}

DEF_MSG_ACK_DEFINE_FUN(ServerConHandler, MSG_SIM_DB2GM_PUSHSERVERID)
{
	if(message->GetErrno() == eReqTimeOut)
	{
		LOG_WARNING(FILEINFO, "gameserver request dbserver push server id and ack timeout");

		return;
	}

	LOG_DEBUG(FILEINFO, "dbServer push server id");

	ServerReturn::ServerRetInt sim;
	int len = 0;
	char * content = message->GetBuffer(len);
	Smart_Ptr<SvrItem> item;

	sim.ParseFromArray(content, len);

//	GUARD(CSimLock, obj, &m_allLock);
	map<int,Smart_Ptr<SvrItem> >::iterator it = m_allSvr.find(message->GetChannelID());
	if(it != m_allSvr.end())
	{
		it->second->serverID = sim.ret();
		item = it->second;
	}
//	obj.UnLock();

//	GUARD(CSimLock, objdb, &m_dbLock);
	m_dbSvr[sim.ret()] = item;
//	objdb.UnLock();
}

DEF_MSG_ACK_DEFINE_FUN(ServerConHandler, MSG_REQ_GM2WS_PUSHSERVERID)
{
	if(message->GetErrno() == eReqTimeOut)
	{
		LOG_WARNING(FILEINFO, "gameserver request worldserver push server id and ack timeout");

		return;
	}

	LOG_DEBUG(FILEINFO, "worldserver push server id");

	ServerReturn::ServerRetInt sim;
	int len = 0;
	char *content = message->GetBuffer(len);
	Smart_Ptr<SvrItem> item;

	sim.ParseFromArray(content, len);

	map<int,Smart_Ptr<SvrItem> >::iterator it = m_allSvr.find(message->GetChannelID());
	if(it != m_allSvr.end())
	{
		it->second->serverID = sim.ret();
		item = it->second;
	}

	m_worldSvr[sim.ret()] = item;

	map<int,Smart_Ptr<SvrItem> >::iterator itGate = m_gateSvr.find(sim.ret());
	if(itGate != m_gateSvr.end())
	{
		CSceneMgr::GetInstance()->SendAllMap(itGate->second->remoteID, itGate->second->remoteType, itGate->second->channelID, m_localID, m_localType);
	}
}

DEF_MSG_ACK_DEFINE_FUN(ServerConHandler, MSG_REQ_GM2MS_PUSHSERVERID)
{
	if(message->GetErrno() == eReqTimeOut)
	{
		LOG_WARNING(FILEINFO, "gameserver request managerServer push server id and ack timeout");

		return;
	}

	LOG_DEBUG(FILEINFO, "managerServer push server id");

	ServerReturn::ServerRetInt sim;
	int len = 0;
	char *content = message->GetBuffer(len);
	Smart_Ptr<SvrItem> item;

	sim.ParseFromArray(content, len);

	map<int,Smart_Ptr<SvrItem> >::iterator it = m_allSvr.find(message->GetChannelID());
	if(it != m_allSvr.end())
	{
		it->second->serverID = sim.ret();
		item = it->second;
	}

	m_managerSvr[sim.ret()] = item;
}

void ServerConHandler::GetWorldServerBySvrID(short int id, Smart_Ptr<SvrItem> &worldSvr)
{
//	GUARD(CSimLock, obj, &m_dbLock);
	map<int,Smart_Ptr<SvrItem> >::iterator it = m_worldSvr.begin();
	for(; it!=m_worldSvr.end(); ++it)
	{
		if(id == it->first)
		{
			worldSvr = it->second;
		}
	}
}

void ServerConHandler::GetManagerServerItem(short int id, Smart_Ptr<SvrItem> &managerSvr)
{
	map<int,Smart_Ptr<SvrItem> >::iterator it = m_managerSvr.find(id);
	if(it!=m_managerSvr.end())
		managerSvr = it->second;
}

Smart_Ptr<SvrItem> ServerConHandler::GetManagerServerItem()
{
	map<int,Smart_Ptr<SvrItem> >::iterator it = m_managerSvr.find(m_serverID);
	if(it!=m_managerSvr.end())
		return it->second;

	return NULL;
}

void ServerConHandler::GetGlobalStatistMgrSvr(Smart_Ptr<SvrItem>& mgrSvr)
{
	// 管理服有多个，那么gameserver是跨服server，则不保存数据
	if (1 != m_managerSvr.size())
	{
		return;
	}

	mgrSvr = m_managerSvr.begin()->second;

}

