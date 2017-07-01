/*
 * ServerManager.cpp
 *
 *  Created on: Jun 29, 2017
 *      Author: root
 */
#include "ServerManager.h"
#include "./Log/Logger.h"
#include "SqlInterface/MysqlInterface.h"
#include "MessageStruct/ServerReturnInt.pb.h"
#include "MessageBuild.h"

ServerConHandler *ServerConHandler::m_instance = 0;

ServerConHandler::ServerConHandler():m_localID(-1),m_localType(0),m_serverID(0)
{

}

ServerConHandler::~ServerConHandler()
{
	m_loginSvr.clear();
	m_gameSvr.clear();
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
	case eLoginServer:
		{
			int key = (remote_type << 16) | remote_id;

			GUARD(CSimLock, obj, &m_loginLock);
			map<int,Safe_Smart_Ptr<SvrItem> >::iterator it = m_loginSvr.find(key);
			if(it != m_loginSvr.end())
			{
				LOG_INFO(FILEINFO,"LoginServer new channel build again and closed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

				Message_Facade::CloseChannel(channel_id);

				break;
			}

			LOG_INFO(FILEINFO,"LoginServer new channel build [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

			Safe_Smart_Ptr<SvrItem> item = new SvrItem();

			item->channelID = channel_id;
			item->ip = remote_address->GetIPToChar();
			item->remoteID = remote_id;
			item->remoteType = remote_type;

			m_loginSvr[key] = item;
			break;
		}
	case eGameServer:
	{
		int key = (remote_type << 16) | remote_id;

		GUARD(CSimLock, obj, &m_gameLock);
		map<int,Safe_Smart_Ptr<SvrItem> >::iterator it = m_gameSvr.find(key);
		if(it != m_gameSvr.end())
		{
			LOG_INFO(FILEINFO,"GameServer new channel build again and closed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

			Message_Facade::CloseChannel(channel_id);

			break;
		}

		LOG_INFO(FILEINFO,"GameServer new channel build [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

		Safe_Smart_Ptr<SvrItem> item = new SvrItem();

		item->channelID = channel_id;
		item->ip = remote_address->GetIPToChar();
		item->remoteID = remote_id;
		item->remoteType = remote_type;
		m_gameSvr[key] = item;
		break;
	}
	default:
		{
			LOG_WARNING(FILEINFO,"unknown new channel build [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());
			Message_Facade::CloseChannel(channel_id);
			break;
		}
	}
}

void ServerConHandler::on_channel_error(int channel_id,short int local_id,unsigned char local_type,short int remote_id,unsigned char remote_type,int error_code,Safe_Smart_Ptr<Inet_Addr> remote_address)
{
	switch(remote_type)
	{
	case eLoginServer:
	{
		int key = (remote_type << 16) | remote_id;

		GUARD(CSimLock, obj, &m_loginLock);
		map<int,Safe_Smart_Ptr<SvrItem> >::iterator it = m_loginSvr.find(key);
		if(it != m_loginSvr.end())
		{
			m_loginSvr.erase(it);
		}

		LOG_INFO(FILEINFO,"LoginServer channel build error [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());
		Message_Facade::CloseChannel(channel_id);
		break;
	}
	case eGameServer:
	{
		int key = (remote_type << 16) | remote_id;

		GUARD(CSimLock, obj, &m_gameLock);
		map<int,Safe_Smart_Ptr<SvrItem> >::iterator it = m_gameSvr.find(key);
		if(it != m_gameSvr.end())
		{
			m_gameSvr.erase(it);
		}

		LOG_INFO(FILEINFO,"GameServer channel build error [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());
		Message_Facade::CloseChannel(channel_id);
		break;
	}
	default:
		{
			LOG_WARNING(FILEINFO,"unknown channel build error [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());
			Message_Facade::CloseChannel(channel_id);
			break;
		}
	}
}

void ServerConHandler::on_connect_failed(int connector_id,short int local_id,unsigned char local_type,short int remote_id,unsigned char remote_type,int error,Safe_Smart_Ptr<Inet_Addr> remote_address)
{
	switch(remote_type)
	{
	case eLoginServer:
		{
			LOG_INFO(FILEINFO,"LoginServer channel build failed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",connector_id, remote_id, remote_type, remote_address->GetIPToChar());

			break;
		}
	case eGameServer:
		{
			LOG_INFO(FILEINFO,"GameServer channel build failed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",connector_id, remote_id, remote_type, remote_address->GetIPToChar());

			break;
		}
	default:
		{
			LOG_WARNING(FILEINFO,"unknown channel build failed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",connector_id, remote_id, remote_type, remote_address->GetIPToChar());

			break;
		}
	}
}

void ServerConHandler::Handle_Request(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_DB2GM_PUSHSERVERID);

	DEF_SWITCH_TRY_DISPATCH_END
}

DEF_MSG_REQUEST_DEFINE_FUN(ServerConHandler, MSG_REQ_DB2GM_PUSHSERVERID)
{
	ServerReturn::ServerRetInt meContent;
	meContent.set_ret(m_serverID);

	Safe_Smart_Ptr<CommBaseOut::Message> messageRet = build_message(MSG_REQ_DB2GM_PUSHSERVERID, message, &meContent);
	Message_Facade::Send(messageRet);
}

