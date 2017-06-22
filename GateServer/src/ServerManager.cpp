/*
 * ServerManager.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "ServerManager.h"
#include "util.h"
#include "ServerMsgDefine.h"
#include "Network/MessageManager.h"
#include "util.h"
#include "MessageBuild.h"
#include "MsgCommonClass.h"
#include "Log/Logger.h"
#include "SvrConfig.h"

ServerConHandler *ServerConHandler::m_instance = 0;

ConnTimeout::ConnTimeout()
{

}

ConnTimeout::~ConnTimeout()
{
	m_timeout.clear();
}

int ConnTimeout::GetGSChannelIDByCharID(const int64 &charid)
{
	GUARD_READ(CRWLock, obj, &m_timeLock);
	map<int64, Safe_Smart_Ptr<ConnClient> >::iterator it = m_timeout.find(charid);
	if(it != m_timeout.end() && it->second.Get())
	{
		return it->second->gsChannelID;
	}

	return -1;
}

bool ConnTimeout::GetClientChannelByCharID(const int64 &charid, int & channel)
{
	GUARD_READ(CRWLock, obj, &m_timeLock);
	map<int64, Safe_Smart_Ptr<ConnClient> >::iterator it = m_timeout.find(charid);
	if(it != m_timeout.end() && it->second.Get())
	{
		channel = it->second->channeid;

		return true;
	}

	return false;
}

bool ConnTimeout::GetGSChannelAndCharIDByChannel(const int &channel, int & gsChannel, int64 &charid)
{
	GUARD_READ(CRWLock, obj, &m_timeLock);
	map<int64, Safe_Smart_Ptr<ConnClient> >::iterator it = m_timeout.begin();
	for(; it != m_timeout.end(); ++it)
	{
		if(it->second.Get() && it->second->channeid == channel)
		{
			gsChannel = it->second->gsChannelID;
			charid = it->second->charid;

			return true;
		}
	}

	return false;
}

bool ConnTimeout::UpdatePlayerInfo(int64 charid, int channelid, string &ip)
{
	GUARD_WRITE(CRWLock, obj, &m_timeLock);
	map<int64, Safe_Smart_Ptr<ConnClient> >::iterator it = m_timeout.find(charid);
	if(it != m_timeout.end() && it->second.Get())
	{
		it->second->channeid = channelid;
		it->second->ip = ip;
		it->second->gsChannelID = Message_Facade::BindGroupChannel(channelid, it->second->group);
	}
	else
	{
		return false;
	}

	return true;
}

bool ConnTimeout::AddTimeout(const int64 &charid, const DWORD64 &time, const int64 &mapid, const WORD &gsid, const int &group)
{
	if(charid < 0)
		return false;

	Safe_Smart_Ptr<ConnClient> client = new ConnClient;

	client->gsID = gsid;
	client->mapID = mapid;
	client->time = time;
	client->charid = charid;
	client->group = group;
	client->gsChannelID = group;//Message_Facade::BindGroupChannel(charid, group);

	GUARD_WRITE(CRWLock, obj, &m_timeLock);;
	map<int64, Safe_Smart_Ptr<ConnClient> >::iterator it = m_timeout.find(charid);
	if(it == m_timeout.end())
	{
		m_timeout[charid] = client;
	}
	else
	{
		return false;
	}
	obj.UnLock();

	return true;
}

void ConnTimeout::removePlayerByGsID(WORD gsId)
{
	vector<int> rVec;

	GUARD_WRITE(CRWLock, obj, &m_timeLock);
	map<int64, Safe_Smart_Ptr<ConnClient> >::iterator it = m_timeout.begin();
	for(; it!=m_timeout.end();)
	{
		if(it->second.Get()==NULL)
		{
			++it;
			continue;
		}

		if(it->second->gsID==gsId)
		{
			Message_Facade::UnBindGroupChannel(it->second->charid, it->second->group);
			if(it->second->channeid >= 0)
			{
				rVec.push_back(it->second->channeid);

				it->second->channeid = -1;
			}

			m_timeout.erase(it++);
		}
		else
		{
				++it;
		}
	}
	obj.UnLock();

	vector<int>::iterator itRem = rVec.begin();
	for(; itRem!=rVec.end(); ++itRem)
	{
		LOG_WARNING(FILEINFO, "close channel[%d]", *itRem);
		Message_Facade::CloseChannel(*itRem);
		ServerConHandler::GetInstance()->closeClientChannel(*itRem);
	}
}


void ConnTimeout::SetClientInfo(const int64  &charid, Safe_Smart_Ptr<sClientItem> &client)
{
	GUARD_READ(CRWLock, obj, &m_timeLock);
	map<int64, Safe_Smart_Ptr<ConnClient> >::iterator it = m_timeout.find(charid);
	if(it != m_timeout.end() && it->second.Get())
	{
		client->channelID = it->second->channeid;
		client->gsID = it->second->gsID;
		client->charID = charid;
		client->mapID = it->second->mapID;
		client->ip = it->second->ip;
	}
}

void ConnTimeout::SynchHeartBeat(const int64 &charid)
{
	GUARD_WRITE(CRWLock, obj, &m_timeLock);
	map<int64, Safe_Smart_Ptr<ConnClient> >::iterator it = m_timeout.find(charid);
	if(it != m_timeout.end() && it->second.Get())
	{
		if(!it->second->isDel)
		{
			DWORD64 now = CUtil::GetNowSecond();
			it->second->time = now + CLIENT_HEART_BEAT;
			LOG_WARNING(FILEINFO, " player[%lld] channel[%d] heart beat next time[%lld] ", it->second->charid, it->second->channeid, it->second->time);
		}
	}
}

bool ConnTimeout::PlayerLogin(const int64 &charid, Safe_Smart_Ptr<ConnClient> &client)
{
	GUARD_WRITE(CRWLock, obj, &m_timeLock);
	map<int64, Safe_Smart_Ptr<ConnClient> >::iterator it = m_timeout.find(charid);
	if(it != m_timeout.end() && it->second.Get())
	{
		DWORD64 now = CUtil::GetNowSecond();
		it->second->time = now + CLIENT_HEART_BEAT;
		client = it->second;
	}
	else
	{
		return false;
	}

	return true;
}

bool ConnTimeout::DelTimeout(const int64  &charid)
{
	GUARD_WRITE(CRWLock, obj, &m_timeLock);
	map<int64, Safe_Smart_Ptr<ConnClient> >::iterator it = m_timeout.find(charid);
	if(it != m_timeout.end() && it->second.Get()!=NULL)
	{
		Message_Facade::UnBindGroupChannel(it->second->charid, it->second->group);
		if(it->second->channeid >= 0)
		{
			LOG_WARNING(FILEINFO, " player[%lld] close channel[%d] from timeout list", it->second->charid, it->second->channeid);
			Message_Facade::CloseChannel(it->second->channeid);
			ServerConHandler::GetInstance()->closeClientChannel(it->second->channeid);
			it->second->channeid = -1;
		}

		m_timeout.erase(it);

		return true;
	}

	return false;
}

//添加一个连接
	void ConnTimeout::clientConnect(int channel,int64 time)
	{
			GUARD_WRITE(CRWLock, obj, &m_connectLock);
			m_connectList[channel] = time;
	}

	//移除连接
	void ConnTimeout::removeConnect(int channel)
	{
			GUARD_WRITE(CRWLock, obj, &m_connectLock);
			std::map<int,int64>::iterator it=m_connectList.find(channel);
			if(it!=m_connectList.end())
			{
					m_connectList.erase(it);
			}
	}

	void ConnTimeout::checkConnect()
	{
			GUARD_WRITE(CRWLock, obj, &m_connectLock);
			int64 curTime = CUtil::GetNowSecond();
			std::map<int,int64>::iterator it=m_connectList.begin();
			for(; it!=m_connectList.end(); )
			{
					if(it->second <= curTime)
					{
							LOG_WARNING(FILEINFO, "close error connected channel[%d]", it->first);
							Message_Facade::CloseChannel(it->first);
							m_connectList.erase(it++);
					}
					else
					{
						++it;
					}
			}
	}

int ConnTimeout::svr()
{
	while(!m_flag)
	{
		//检测连接有效性
		checkConnect();

		//检测登录超时
		vector<int> delMap;
		vector<int64> lineDel;
		{
			GUARD_WRITE(CRWLock, obj, &m_timeLock);
			if(m_timeout.size() == 0)
			{
				obj.UnLock();
				sleep(5);

				continue;
			}

			DWORD64 now = CUtil::GetNowSecond();

			map<int64, Safe_Smart_Ptr<ConnClient> >::iterator it = m_timeout.begin();
			for(; it != m_timeout.end(); )
			{
				if(it->second.Get()==NULL)
				{
					++it;
					continue;
				}

				if(it->second->time <= now && !it->second->isDel)
				{
					if(it->second->gsID > 0)
					{
						LOG_WARNING(FILEINFO, "player[%lld] timeout and client channel[%d] and to GameServer login out", GET_PLAYER_CHARID(it->second->charid), it->second->channeid);
						if(it->second->channeid >= 0)
						{
							delMap.push_back(it->second->channeid);
							it->second->isDel = true;
							it->second->time = now + 15000;
						}
						else
						{
							m_timeout.erase(it++);

							continue;
						}
					}
				}
				else if(it->second->time <= now && it->second->isDel)
				{
					LOG_WARNING(FILEINFO, "player[%lld] timeout and client channel[%d] and login out", GET_PLAYER_CHARID(it->second->charid), it->second->channeid);
					lineDel.push_back(it->second->charid);
				}

				++it;
			}
		}

		vector<int>::iterator itDel = delMap.begin();
		for(; itDel != delMap.end(); ++itDel)
		{
			LOG_WARNING(FILEINFO,"ServerConHandler::PlayerExit(const int &channel, bool flag)...............channel[%d]...................", *itDel);
			ServerConHandler::GetInstance()->PlayerExit(*itDel, 1);
		}

		vector<int64>::iterator itLine = lineDel.begin();
		for(; itLine != lineDel.end(); ++itLine)
		{
			LOG_WARNING(FILEINFO,"ServerConHandler::PlayerExitEx(const int64 &charid, Safe_Smart_Ptr<Message> message).................................. %lld", GET_PLAYER_CHARID(*itLine));
			ServerConHandler::GetInstance()->PlayerExitEx(*itLine, 0);
		}

		sleep(5);
	}

	return 0;
}

ServerConHandler::ServerConHandler():m_localID(-1),m_localType(0),m_port(0),m_serverID(0)
{
	if(m_timeout.Start(1))
	{
		LOG_ERROR(FILEINFO, "GateServer start timeout Thread error");
	}

	m_client.resize(MAX_EPOLL_fd,NULL);
}

ServerConHandler::~ServerConHandler()
{
	m_timeout.Close();
	m_client.clear();
	m_idClient.clear();
	m_gsIDSvr.clear();
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
	case eClient:
		{
			int64 timeOut = CUtil::GetNowSecond()+60000;
			clientConnect(channel_id,timeOut);
			LOG_INFO(FILEINFO,"ClientServer new channel build [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());
			break;
		}
	case eGameServer:
		{
			unsigned int gsID = CREATE_GSID(local_id, remote_id);

			GUARD_WRITE(CRWLock, obj, &m_gsIDLock);
			map<WORD, Safe_Smart_Ptr<SvrItem> >::iterator it =  m_gsIDSvr.find(gsID);
			if(it != m_gsIDSvr.end())
			{
				LOG_WARNING(FILEINFO,"GameServer new channel build again closed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

				Message_Facade::CloseChannel(channel_id);

				break;
			}

			LOG_INFO(FILEINFO,"GameServer new channel build [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

			Safe_Smart_Ptr<SvrItem> item = new SvrItem();

			item->channelID = channel_id;
			item->ip = remote_address->GetIPToChar();
			item->remoteID = remote_id;
			item->remoteType = remote_type;
			m_gsIDSvr[gsID] = item;

			break;
		}
	case eLoginServer:
		{
			if(m_loginServer.channelID != -1)
			{
				LOG_WARNING(FILEINFO,"LoginServer channel build again and closed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());
				Message_Facade::CloseChannel(channel_id);

				break;
			}

			LOG_INFO(FILEINFO,"LoginServer new channel build [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

			m_loginServer.channelID = channel_id;
			m_loginServer.ip = remote_address->GetIPToChar();
			m_loginServer.remoteID = remote_id;
			m_loginServer.remoteType = remote_type;

			break;
		}
	case eWorldServer:
		{
			if(m_worldServer.channelID != -1)
			{
				LOG_WARNING(FILEINFO,"WorldServer channel build again and closed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());
				Message_Facade::CloseChannel(channel_id);

				break;
			}

			LOG_INFO(FILEINFO,"WorldServer new channel build [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",channel_id, remote_id, remote_type, remote_address->GetIPToChar());

			m_worldServer.channelID = channel_id;
			m_worldServer.ip = remote_address->GetIPToChar();
			m_worldServer.remoteID = remote_id;
			m_worldServer.remoteType = remote_type;

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
	case eClient:
		{
			LOG_INFO(FILEINFO,"client new channel build error [channelid = %d, remoteid=%d, remotetype=%d, ip=%s , errorno=%d]",channel_id, remote_id, remote_type, remote_address->GetIPToChar(), error_code);
			LOG_WARNING(FILEINFO,"ServerConHandler::PlayerExitEx(const int &channel, bool flag)..................................");
			PlayerExit(channel_id, 1);

			break;
		}
	case eGameServer:
		{
			WORD gsID = CREATE_GSID(local_id, remote_id);
			vector<int> copyID;

			GUARD_WRITE(CRWLock, obj, &m_gsIDLock);
			map<WORD, Safe_Smart_Ptr<SvrItem> >::iterator it =  m_gsIDSvr.find(gsID);
			if(it != m_gsIDSvr.end())
			{
				m_gsIDSvr.erase(it);
			}
			obj.UnLock();

			//函数有锁，外面不要加锁
			ServerConHandler::GetInstance()->clearPlayerFromServer(gsID);
			Message_Facade::CloseChannel(channel_id);

			LOG_INFO(FILEINFO,"GameServer new channel build error [channelid = %d, remoteid=%d, remotetype=%d, ip=%s , errorno=%d]",channel_id, remote_id, remote_type, remote_address->GetIPToChar(), error_code);

			break;
		}
	case eLoginServer:
		{
			LOG_INFO(FILEINFO,"LoginServer new channel build error [channelid = %d, remoteid=%d, remotetype=%d, ip=%s , errorno=%d]",channel_id, remote_id, remote_type, remote_address->GetIPToChar(), error_code);
			m_loginServer.clear();
			Message_Facade::CloseChannel(channel_id);

			break;
		}

	case eWorldServer:
	{
		LOG_INFO(FILEINFO,"WorldServer new channel build error [channelid = %d, remoteid=%d, remotetype=%d, ip=%s , errorno=%d]",channel_id, remote_id, remote_type, remote_address->GetIPToChar(), error_code);

		m_worldServer.clear();
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
	case eClient:
		{
			LOG_INFO(FILEINFO,"client new channel build failed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",connector_id, remote_id, remote_type, remote_address->GetIPToChar());

			break;
		}
	case eGameServer:
		{
			LOG_INFO(FILEINFO,"GameServer new channel build failed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",connector_id, remote_id, remote_type, remote_address->GetIPToChar());
			break;
		}
	case eLoginServer:
		{
			LOG_INFO(FILEINFO,"LoginServer new channel build failed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",connector_id, remote_id, remote_type, remote_address->GetIPToChar());

			break;
		}
	case eWorldServer:
	{
		LOG_INFO(FILEINFO,"WorldServer new channel build failed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s , errorno=%d]",connector_id, remote_id, remote_type, remote_address->GetIPToChar());

		break;
	}
	default:
		{
			LOG_WARNING(FILEINFO,"unknown channel build failed [channelid = %d, remoteid=%d, remotetype=%d, ip=%s]",connector_id, remote_id, remote_type, remote_address->GetIPToChar());
			break;
		}
	}
}

int64 ServerConHandler::GetCharidByChannel(int channelid)
{
	GUARD_READ(CRWLock, obj, &m_clientMutex);
	if(channelid < 0 || channelid>=(int)m_client.size())
		return 0;

	Safe_Smart_Ptr<sClientItem> &ref= m_client[channelid];
	if(ref.Get())
	{
		return ref->charID;
	}
	else
	{
		obj.UnLock();

		return m_timeout.GetCharIDByChannelID(channelid);
	}

	return 0;
}

void ServerConHandler::UpdateSceneidByChar(const int64 &charid,const int64 &sceneid, const int &gsid)
{
	int channel_id = -1;
	int group = -1;

	GUARD_READ(CRWLock, objgs, &m_gsIDLock);
	map<WORD, Safe_Smart_Ptr<SvrItem> >::iterator itGs = m_gsIDSvr.find(CREATE_GSID(m_localID, gsid));
	if(itGs != m_gsIDSvr.end() && itGs->second.Get()!=NULL)
	{
		group = itGs->second->channelID;
	}
	objgs.UnLock();

	if(group==-1)
		return;

	m_timeout.ChangeMapUpdateInfo(charid, channel_id, CREATE_GSID(m_localID, gsid), group);
}

int64 ServerConHandler::GetSceneidByChannel(int channelid)
{
	GUARD_READ(CRWLock, obj, &m_clientMutex);
	if(channelid < 0 || channelid>=(int)m_client.size())
			return 0;

	Safe_Smart_Ptr<sClientItem> &ref = m_client[channelid];
	if(ref.Get())
	{
		return ref->mapID;
	}

	return 0;
}

bool ServerConHandler::GetGSChannelByCharID(const int64 &charid, int &channel)
{
	GUARD_READ(CRWLock, obj, &m_clientLock);
	map<int64, Safe_Smart_Ptr<sClientItem> >::iterator it =  m_idClient.find(charid);
	if(it != m_idClient.end() && it->second.Get()!=NULL)
	{
		channel = it->second->gsChannelID;
		if(channel < 0)
			return false;
	}
	else
	{
		obj.UnLock();

		channel = m_timeout.GetGSChannelIDByCharID(charid);
		if(channel < 0)
			return false;
	}

	return true;
}

bool ServerConHandler::GetClientChannelByCharID(const int64 &charid, int &channel)
{
	{
		GUARD_READ(CRWLock, obj, &m_clientLock);
		map<int64, Safe_Smart_Ptr<sClientItem> >::iterator it =  m_idClient.find(charid);
		if(it != m_idClient.end() && it->second.Get()!=NULL)
		{
			channel = it->second->channelID;
			if(channel < 0)
			{
				return false;
			}
		}
		else
		{
			obj.UnLock();

			return m_timeout.GetClientChannelByCharID(charid, channel);
		}
	}

	return true;
}

bool ServerConHandler::GetGSChannelByChannel(const int &channel, int & gsChannel)
{
	int64 charid = 0;
	if(channel < 0 || channel>=(int)m_client.size())
			return false;

	GUARD_READ(CRWLock, obj, &m_clientMutex);
	Safe_Smart_Ptr<sClientItem> &ref = m_client[channel];
	if(ref.Get())
	{
		gsChannel = ref->gsChannelID;
		if(gsChannel < 0)
			return false;
	}
	else
	{
		obj.UnLock();

		return m_timeout.GetGSChannelAndCharIDByChannel(channel, gsChannel, charid);
	}

	return true;
}

bool ServerConHandler::GetGSChannelAndCharIDByChannel(const int &channel, int & gsChannel, int64 &charid)
{
	if(channel < 0 || channel>=(int)m_client.size())
			return false;

	GUARD_READ(CRWLock, obj, &m_clientMutex);
	Safe_Smart_Ptr<sClientItem> &ref = m_client[channel];
	if(ref.Get())
	{
		gsChannel = ref->gsChannelID;
		charid = ref->charID;
		if(gsChannel < 0)
			return false;
	}
	else
	{
		obj.UnLock();

		return m_timeout.GetGSChannelAndCharIDByChannel(channel, gsChannel, charid);
	}

	return true;
}

bool ServerConHandler::GetGSChannelAndCharIDByChannelEx(const int &channel, int & gsChannel, int64 &charid)
{
		return m_timeout.GetGSChannelAndCharIDByChannel(channel, gsChannel, charid);
}

void ServerConHandler::clearPlayerFromServer(WORD gsID)
{
//	ServerReturn::ServerRetIntArray ret;

	GUARD_WRITE(CRWLock, obj, &m_clientLock);
	map<int64, Safe_Smart_Ptr<sClientItem> >::iterator itID = m_idClient.begin();
	for(;itID!=m_idClient.end();)
	{
		if(itID->second.Get()==NULL)
		{
			++itID;

			continue;
		}
		if(itID->second->gsID==gsID)
		{
//			 ret.add_ret(itID->first);
			 m_idClient.erase(itID++);
		}
		else
		{
			++itID;
		}
	}
	obj.UnLock();

	m_timeout.removePlayerByGsID(gsID);

//	if(ret.ret_size()>0)
//	{
//		Safe_Smart_Ptr<CommBaseOut::Message> message = build_message(MSG_SIM_GT2LS_KICKOFF_PLAYERS, &ret, &m_loginServer, SimpleMessage);
//		Message_Facade::Send(message);
//	}
}

void ServerConHandler::PlayerExitEx(const int64 &charid, Safe_Smart_Ptr<Message> message)
{
	GUARD_WRITE(CRWLock, obj, &m_clientLock);
	map<int64, Safe_Smart_Ptr<sClientItem> >::iterator itID = m_idClient.find(charid);
	if(itID != m_idClient.end())
	{
		unsigned int serverid = 0;
		int channel_id = 0;
		int64 mapid = 0;

		if(itID->second.Get()!=NULL)
		{
			serverid = itID->second->gsID;
			channel_id = itID->second->channelID;
			mapid = itID->second->mapID;
		}

		m_idClient.erase(itID);
		obj.UnLock();

		m_timeout.DelTimeout(charid);

//		ServerReturn::ServerRetInt pathcharid;
//
//		pathcharid.set_ret(charid);
   if(message)
		{
//			Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_LG2GT_PLAYER_EXIT, message, &pathcharid);
//			Message_Facade::Send(messRet);
		}
   else
	   {
//	   ServerReturn::ServerDoubleInt meContent;
//
//	   meContent.set_retf(charid);
//	   meContent.set_rets((m_localType << 16) | m_localID);
//
//	   Safe_Smart_Ptr<CommBaseOut::Message> message = build_message(MSG_SIM_GT2LS_CLIENTBREAK, &meContent, &m_loginServer, SimpleMessage);
//	   Message_Facade::Send(message);
	   }

   GUARD_WRITE(CRWLock, objgs, &m_gsIDLock);
		map<WORD, Safe_Smart_Ptr<SvrItem> >::iterator itGs = m_gsIDSvr.find(serverid);
		if(itGs != m_gsIDSvr.end() && itGs->second.Get()!=NULL)
		{
			map<int64, int>::iterator itClient = itGs->second->mapPlayer.find(mapid);
			if(itClient != itGs->second->mapPlayer.end())
			{
				--itClient->second;
				if(itClient->second < 0)
					itClient->second = 0;
			}
			else
			{
				LOG_WARNING(FILEINFO,"mapid about player find not map id [charid = %lld ,channelid = %d]", GET_PLAYER_CHARID(charid),channel_id);
			}
		}
		else
		{
			LOG_WARNING(FILEINFO,"gameid about player find not gameserver id [charid = %lld ,channelid = %d]", GET_PLAYER_CHARID(charid),channel_id);
		}
		objgs.UnLock();
	}
	else
	{
		obj.UnLock();

		m_timeout.DelTimeout(charid);

		//返回消息给登录服
		if(message)
		{
//			ServerReturn::ServerRetInt pathcharid;
//
//			pathcharid.set_ret(charid);
//
//			Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_LG2GT_PLAYER_EXIT, message, &pathcharid);
//			Message_Facade::Send(messRet);
		}
	}
}

void ServerConHandler::PlayerExit(const int64 &charid, Safe_Smart_Ptr<Message> message)
{
	int gsChannel = -1;

	if(GetGSChannelByCharID(charid, gsChannel))
	{
//		ServerReturn::ServerDoubleInt gamecharid;
//
//		gamecharid.set_retf(charid);
//		gamecharid.set_rets(static_cast<commonAct *>(message->GetRequest()->GetAct().Get())->mesDatas);
//
//		Safe_Smart_Ptr<CommBaseOut::Message> gamemess = build_message(MSG_REQ_GT2GM_PLAYEREXIT, &gamecharid, gsChannel, Request);
//		gamemess->SetAct(new FirstAct(static_cast<commonAct *>(message->GetRequest()->GetAct().Get())->mesReq, charid));
//		Message_Facade::Send(gamemess);
	}
	else
	{
		if((bool)static_cast<commonAct *>(message->GetRequest()->GetAct().Get())->mesReq)
		{
//			ServerReturn::ServerRetInt ret;
//
//			ret.set_ret(charid);
//
//			Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_LG2GT_PLAYER_EXIT, static_cast<commonAct *>(message->GetRequest()->GetAct().Get())->mesReq, &ret);
//			Message_Facade::Send(messRet);
		}
	}
}

void ServerConHandler::PlayerExit(const int &channel, bool flag)
{
	if(channel<0 || channel>=(int)m_client.size())
		return;

	int64 charid = 0;
	Safe_Smart_Ptr<CommBaseOut::Message> message;
	int gsChannel = -1;

	if(GetGSChannelAndCharIDByChannelEx(channel, gsChannel, charid))
	{
//		ServerReturn::ServerDoubleInt gamecharid;
//
//		gamecharid.set_retf(charid);
//		gamecharid.set_rets((m_localType << 16) | m_localID);
//
//   Safe_Smart_Ptr<CommBaseOut::Message> mesWorld = build_message(MSG_SIM_GT2WS_PLAYEREXIT, &gamecharid, &m_worldServer, Request);
//		mesWorld->SetAct(new commonAct(message, charid, flag));
//   Message_Facade::Send(mesWorld);
	}
	else
	{
		LOG_WARNING(FILEINFO, "player exit and close channel[%d]", channel);
		Message_Facade::CloseChannel(channel);
	}
}

void ServerConHandler::PlayerExit(const int64 &charid, bool flag, Safe_Smart_Ptr<Message> message)
{
//	if(m_timeout.IsInTimeoutList(charid))
//	{
//		ServerReturn::ServerDoubleInt gamecharid;
//
//		gamecharid.set_retf(charid);
//   gamecharid.set_rets((m_localType << 16) | m_localID);
//
//   Safe_Smart_Ptr<CommBaseOut::Message> mesWorld = build_message(MSG_SIM_GT2WS_PLAYEREXIT, &gamecharid, &m_worldServer, Request);
//		mesWorld->SetAct(new commonAct(message, charid, flag));
//   Message_Facade::Send(mesWorld);
//	}
//	else
//	{
//		if(message)
//		{
//			ServerReturn::ServerRetInt ret;
//
//			ret.set_ret(charid);
//
//			Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_LG2GT_PLAYER_EXIT, message, &ret);
//			Message_Facade::Send(messRet);
//		}
//	}
}


bool ServerConHandler::UpdateClientInfo(int64 charid, int channelid, string ip)
{
	return m_timeout.UpdatePlayerInfo(charid, channelid, ip);
}

bool ServerConHandler::AddClientTimeOut(DWORD64 charid, Safe_Smart_Ptr<CommBaseOut::Message> &message)
{
	Safe_Smart_Ptr<SvrItem> gaSvr = ServerConHandler::GetLeastGameServer();
	if(gaSvr)
	{
		if(!m_timeout.AddTimeout(charid, CUtil::GetNowSecond() + CONN_GATESERVER_TIMEOUT*1000, 0, CREATE_GSID(m_localID, gaSvr->remoteID), gaSvr->channelID))
		{
			LOG_ERROR(FILEINFO, "player is logining , so add timeout error");

			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

void ServerConHandler::SynchHeartBeat(const int &channel)
{
	if(channel < 0 || channel>=(int)m_client.size())
		return;

	GUARD_READ(CRWLock, objChannel, &m_clientMutex);
	Safe_Smart_Ptr<sClientItem> &ref = m_client[channel];
	if(ref.Get())
	{
		int64 charid = ref->charID;

		objChannel.UnLock();

		m_timeout.SynchHeartBeat(charid);
	}
	else
	{
		LOG_ERROR(FILEINFO, "heart beat synch and gateserver not find client [ channel = %d]", channel);
	}
}

//客户端连接上
void ServerConHandler::clientConnect(int channel, int64 time)
{
	m_timeout.clientConnect(channel,time);
}

	//移除连接
void ServerConHandler::removeConnect(int channel)
{
	m_timeout.removeConnect(channel);
}


bool ServerConHandler::PlayerLogin(const int64 &charid, const int64 & mapid)
{
//	Safe_Smart_Ptr<ConnTimeout::ConnClient> client;
//	int64 oldMap = -1;
//	int gsid = -1;
//	bool isSame = false;
//
//	LOG_WARNING(FILEINFO,"ServerConHandler::PlayerLogin(const int64 &charid, Safe_Smart_Ptr<ConnClient> &client).................................. %lld", GET_PLAYER_CHARID(charid));
//	if(!m_timeout.PlayerLogin(charid, client))
//	{
//		return false;
//	}
//
//	oldMap = client->mapID;
//	client->mapID = mapid;
//
//	GUARD_WRITE(CRWLock, obj, &m_clientLock);
//	map<int64, Safe_Smart_Ptr<sClientItem> >::iterator itItem =  m_idClient.find(charid);
//	if(itItem != m_idClient.end() && itItem->second.Get()!=NULL)
//	{
//		itItem->second->group = client->group;
//		itItem->second->gsChannelID = client->gsChannelID;
//		gsid = itItem->second->gsID;
//		itItem->second->gsID = client->gsID;
//		itItem->second->mapID = mapid;
//		if(gsid == client->gsID)
//			isSame = true;
//
//		obj.UnLock();
//	}
//	else
//	{
//		Safe_Smart_Ptr<sClientItem> item = new sClientItem();
//
//		item->channelID = client->channeid;
//		item->group = client->group;
//		item->gsChannelID = client->gsChannelID;
//		item->charID = client->charid;
//		item->gsID = client->gsID;
//		item->ip = client->ip;
//		item->mapID = mapid;
//
//		m_idClient[item->charID] = item;
//		obj.UnLock();
//
//		GUARD_WRITE(CRWLock, objChannel, &m_clientMutex);
//		if(item->channelID >=0 && item->channelID<(int)m_client.size())
//		{
//			m_client[item->channelID] = item;
//		}
//	}
//
//	bool isHave = IsHaveSceneID(mapid);
//
//	GUARD_WRITE(CRWLock, objgs, &m_gsIDLock);
//	map<WORD, Safe_Smart_Ptr<SvrItem> >::iterator it = m_gsIDSvr.find(client->gsID);
//	if(it != m_gsIDSvr.end() && it->second.Get()!=NULL)
//	{
//		map<int64, int>::iterator itmap = it->second->mapPlayer.find(mapid);
//		if(itmap != it->second->mapPlayer.end())
//		{
//			++itmap->second;
//		}
//		else
//		{
//			if(isHave)
//			{
//				it->second->mapPlayer[mapid] = 1;
//			}
//		}
//
//		if(isSame)
//		{
//			itmap = it->second->mapPlayer.find(oldMap);
//			if(itmap != it->second->mapPlayer.end())
//			{
//				--itmap->second;
//				if(itmap->second < 0)
//					itmap->second = 0;
//			}
//		}
//	}
//
//	if(!isSame)
//	{
//		it = m_gsIDSvr.find(gsid);
//		if(it != m_gsIDSvr.end() && it->second.Get()!=NULL)
//		{
//			map<int64, int>::iterator itmap = it->second->mapPlayer.find(oldMap);
//			if(itmap != it->second->mapPlayer.end())
//			{
//				--itmap->second;
//				if(itmap->second < 0)
//					itmap->second = 0;
//			}
//		}
//	}
//
	return true;
}

Safe_Smart_Ptr<SvrItem> ServerConHandler::GetLeastGameServer()
{
	int playerNum = 0;
	int ret = -1;
	Safe_Smart_Ptr<SvrItem> gs;

	GUARD_READ(CRWLock, obj, &m_gsIDLock);
	map<WORD, Safe_Smart_Ptr<SvrItem> >::iterator itGs = m_gsIDSvr.begin();
	for(; itGs != m_gsIDSvr.end(); ++itGs)
	{
		if(itGs->second.Get()==NULL)
		{
			continue;
		}

		int num = itGs->second->GetPlayerNum();

		if(ret == -1)
		{
			gs = itGs->second;
			playerNum = num;
			ret = 1;

			continue;
		}

		if(playerNum > num)
		{
			gs = itGs->second;
			playerNum = num;
		}
	}

	return gs;
}

//得到当线玩家人数
int ServerConHandler::GetGsPlayerNum(int serverid)
{
	GUARD_READ(CRWLock, objgs, &m_gsIDLock);
	map<WORD, Safe_Smart_Ptr<SvrItem> >::iterator itGs = m_gsIDSvr.find(serverid);
	if(itGs != m_gsIDSvr.end() && itGs->second.Get()!=NULL)
	{
		return itGs->second->GetPlayerNum();
	}

	return 0;
}

//得到当线当图玩家人数(当前场景)
int ServerConHandler::GetGsMapPlayerNum(int64 sceneid)
{
	int num = 0;

	GUARD_READ(CRWLock, objgs, &m_gsIDLock);
	map<WORD, Safe_Smart_Ptr<SvrItem> >::iterator itGs = m_gsIDSvr.begin();
	for(; itGs!=m_gsIDSvr.end(); ++itGs)
	{
		if(itGs->second.Get()==NULL)
		{
			continue;
		}

		map<int64, int>::iterator it = itGs->second->mapPlayer.find(sceneid);
		if(it != itGs->second->mapPlayer.end())
		{
			num = it->second;

			break;
		}
	}

	LOG_DEBUG(FILEINFO, "当线人数：%d",num);

	return num;
}


void ServerConHandler::Handle_Message(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

//	DEF_MSG_SIMPLE_DISPATCH_FUN(MSG_SIM_GM2GT_PUSHALLMAP);
//	DEF_MSG_SIMPLE_DISPATCH_FUN(MSG_SIM_GM2GT_PLAYER_EXIT);
//	DEF_MSG_SIMPLE_DISPATCH_FUN(MSG_REQ_WS2GT_CREATESCENE);

	DEF_SWITCH_TRY_DISPATCH_END
}

void ServerConHandler::Handle_Request(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

//	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_SIM_GT2GM_PUSHSERVERID);

	DEF_SWITCH_TRY_DISPATCH_END
}

void ServerConHandler::Handle_Ack(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

//	DEF_MSG_ACK_DISPATCH_FUN(MSG_REQ_GT2GM_PLAYEREXIT);

	DEF_SWITCH_TRY_DISPATCH_END
}

//DEF_MSG_ACK_DEFINE_FUN(ServerConHandler, MSG_REQ_GT2GM_PLAYEREXIT)
//{
//	if(message->GetErrno() == eReqTimeOut)
//	{
//		LOG_WARNING(FILEINFO, "gateserver request gameserver exit player and ack timeout");
//	}
//
//	LOG_WARNING(FILEINFO,"ServerConHandler::PlayerExitEx(const int64 &charid, Safe_Smart_Ptr<Message> message).................................. %lld", GET_PLAYER_CHARID(static_cast<FirstAct *>(act.Get())->mesDataf));
//	PlayerExitEx(static_cast<FirstAct *>(act.Get())->mesDataf, static_cast<FirstAct *>(act.Get())->mesReq);
//}

