/*
 * ServerManager.h
 *
 *  Created on: Jun 29, 2017
 *      Author: root
 */

#ifndef SERVERMANAGER_H_
#define SERVERMANAGER_H_

#include "./Network/Service_Handler.h"
#include "./Network/InetAddress.h"
#include "Smart_Ptr.h"
#include "./Network/NetWorkConfig.h"
#include <map>
#include "Common.h"
#include "Ref_Object.h"
#include "ServerMsgDefine.h"
#include "Network/MessageManager.h"
#include "MsgDefineMacro.h"
#include "./Network/MessageHandler.h"
#include "./Network/Message_Facade.h"

using namespace CommBaseOut;
using namespace std;

typedef struct sSvrItem : public Ref_Object
{
	int channelID;
	short int remoteID;
	unsigned char remoteType;
	string ip;
	sSvrItem()
	{
		channelID = -1;
		remoteID = -1;
		remoteType = 0;
	}

	void clear()
	{
		channelID = -1;
		remoteID = -1;
		remoteType = 0;
	}
}SvrItem;

class ServerConHandler:public Message_Service_Handler, public Request_Handler
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
		DEF_MSG_REQUEST_REG_FUN(eDBServer, MSG_REQ_DB2GM_PUSHSERVERID);
	}


	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_DB2GM_PUSHSERVERID);

	virtual void Handle_Request(Safe_Smart_Ptr<Message> &message);

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

	void SetSvrID(int id)
	{
		m_serverID = id;
	}
	int GetSvrID()
	{
		return m_serverID;
	}
private:
	ServerConHandler();

private:
	map<int,Safe_Smart_Ptr<SvrItem> > m_loginSvr;
	CSimLock m_loginLock;
	map<int,Safe_Smart_Ptr<SvrItem> > m_gameSvr;
	CSimLock m_gameLock;
	static ServerConHandler * m_instance;
	short int m_localID;
	unsigned char m_localType;
	int m_serverID;
};

#endif /* SERVERMANAGER_H_ */
