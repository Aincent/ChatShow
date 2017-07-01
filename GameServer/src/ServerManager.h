/*
 * ServerManager.h
 *
 *  Created on: Jun 27, 2017
 *      Author: root
 */

#ifndef SERVERMANAGER_H_
#define SERVERMANAGER_H_

#include "./Network/Service_Handler.h"
#include "./Network/InetAddress.h"
#include "Smart_Ptr.h"
#include "./Network/NetWorkConfig.h"
#include <map>
#include "define.h"
#include "Common.h"
#include "Network/MessageHandler.h"
#include "ServerMsgDefine.h"
#include "MsgDefineMacro.h"
#include "Network/Message_Facade.h"
#include "Ref_Object.h"
#include "./google/protobuf/message.h"

using namespace CommBaseOut;
using namespace std;


typedef struct sSvrItem : public Ref_Object
{
	int channelID;
	short int remoteID;
	unsigned char remoteType;
	string ip;
	int serverID;
	sSvrItem()
	{
		channelID = -1;
		remoteID = -1;
		remoteType = 0;
		serverID = -1;
	}

	void clear()
	{
		channelID = -1;
		remoteID = -1;
		remoteType = 0;
		serverID = -1;
	}
}SvrItem;

class ServerConHandler:public Message_Service_Handler, public Ack_Handler
{
public:

	~ServerConHandler();

	virtual void on_new_channel_build(int channel_id,short int local_id,unsigned char local_type,short int remote_id,unsigned char remote_type,Safe_Smart_Ptr<Inet_Addr> remote_address);

	virtual void on_channel_error(int channel_id,short int local_id,unsigned char local_type,short int remote_id,unsigned char remote_type,int error_code,Safe_Smart_Ptr<Inet_Addr> remote_address);

	virtual void on_connect_failed(int connector_id,short int local_id,unsigned char local_type,short int remote_id,unsigned char remote_type,int error,Safe_Smart_Ptr<Inet_Addr> remote_address);

	static ServerConHandler * GetInstance()
	{
		if(m_instance == 0)
		{
			m_instance = new ServerConHandler();
		}

		return m_instance;
	}

	void RegisterMsg()
	{
		DEF_MSG_ACK_REG_FUN(eGameServer, MSG_REQ_GT2GM_PUSHSERVERID);
		DEF_MSG_ACK_REG_FUN(eGameServer, MSG_REQ_DB2GM_PUSHSERVERID);
//		DEF_MSG_ACK_REG_FUN(eGameServer, MSG_REQ_GM2WS_PUSHSERVERID);
//		DEF_MSG_ACK_REG_FUN(eGameServer, MSG_REQ_GM2MS_PUSHSERVERID);
	}

	DEF_MSG_ACK_DECLARE_FUN_H(MSG_REQ_GT2GM_PUSHSERVERID);
	DEF_MSG_ACK_DECLARE_FUN_H(MSG_REQ_DB2GM_PUSHSERVERID);
//	DEF_MSG_ACK_DECLARE_FUN_H(MSG_REQ_GM2WS_PUSHSERVERID);
//	DEF_MSG_ACK_DECLARE_FUN_H(MSG_REQ_GM2MS_PUSHSERVERID);c

	virtual void Handle_Ack(Safe_Smart_Ptr<Message> &message);

	void DestoryInstance()
	{
		if(m_instance)
		{
			delete m_instance;
			m_instance = 0;
		}
	}

	short int GetLocalID()
	{
		return m_localID;
	}

	unsigned char GetLocalType()
	{
		return m_localType;
	}

	void GetDBServerBySvrID(short int id, Smart_Ptr<SvrItem> &dbSvr);
	//根据serverid获取db的channel id
	int GetDBChannelBySvrID(int id);

	void GetGateServerBySvrID(short int id, Smart_Ptr<SvrItem> &gtSvr);
	void GetAllGateServer(vector<Smart_Ptr<SvrItem> > &gtSvr)
	{
		map<int,Smart_Ptr<SvrItem> >::iterator it = m_gateSvr.begin();
		for(; it!=m_gateSvr.end(); ++it)
		{
			gtSvr.push_back(it->second);
		}
	}
	//得到一个随机GTserver（用于世界广播消息）
	void GetGateServer(Smart_Ptr<SvrItem> &gtSvr);
	//得到pathserver
	Smart_Ptr<SvrItem> & GetPathServer()
	{
		return m_pathSvr;
	}

	void SendMessageToServer(unsigned int msgType, short int rID, unsigned char rType, int channelID, unsigned int msgID, google::protobuf::Message *content)
	{
		Safe_Smart_Ptr<CommBaseOut::Message> newMessage = NEW CommBaseOut::Message(rID, rType, channelID);
		char str[MAX_RECV_MSG_SIZE] = {0};
		content->SerializePartialToArray(str, content->ByteSize());

		newMessage->SetMessageType(msgType);
		newMessage->SetMessageID(msgID);
		newMessage->SetLocalType(ServerConHandler::GetInstance()->GetLocalType());
		newMessage->SetContent(str, content->ByteSize());
		if(msgType == Request)
		{
			newMessage->SetTimeout(5);
		}

		Message_Facade::Send(newMessage);
	}

	//广播消息到世界服
	void BraodcastMessageToWorld(unsigned int msgType, unsigned int msgID, google::protobuf::Message *content);

	void GetWorldServerBySvrID(short int id, Smart_Ptr<SvrItem> &worldSvr);

	void GetManagerServerItem(short int id, Smart_Ptr<SvrItem> &managerSvr);

	Smart_Ptr<SvrItem> GetManagerServerItem();

	// 获取全局统计需要的管理服
	void GetGlobalStatistMgrSvr(Smart_Ptr<SvrItem>& mgrSvr);

	int GetServerId() const
	{
		return m_serverID;
	}

	void SetServerId(int serverId)
	{
		m_serverID = serverId;
	}

private:
	ServerConHandler();

private:

	map<int,Smart_Ptr<SvrItem> > m_gateSvr;
	CSimLock m_gateLock;
	map<int,Smart_Ptr<SvrItem> > m_dbSvr;
	CSimLock m_dbLock;
	map<int,Smart_Ptr<SvrItem> > m_allSvr;
	CSimLock m_allLock;

	//-------------------------------------------------------

	Smart_Ptr<SvrItem> m_pathSvr; //pathserver

	map<int,Smart_Ptr<SvrItem> > m_worldSvr;

	map<int,Smart_Ptr<SvrItem> > m_managerSvr;  //后台管理服务器

	static ServerConHandler * m_instance;
	short int m_localID;
	unsigned char m_localType;

	int m_serverID;
};

#endif /* SERVERMANAGER_H_ */
