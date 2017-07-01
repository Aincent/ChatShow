/*
 * CharLogin.h
 *
 *  Created on: Jun 29, 2017
 *      Author: root
 */

#ifndef CHARLOGIN_H_
#define CHARLOGIN_H_

#include "Smart_Ptr.h"
#include "Network/MessageHandler.h"
#include "Network/MessageManager.h"
#include "ServerMsgDefine.h"
#include "MsgDefineMacro.h"
//#include "MessageStruct/CharLogin/PlayerInfo.pb.h"
#include "MessageCommonRet.h"
#include "SqlInterface/MysqlInterface.h"
//#include "MessageStruct/CharLogin/GetCharID.pb.h"

using namespace CommBaseOut;

class PlayerDBMgr;

class CCharLogin : public Request_Handler, public Message_Handler
{
public:
	~CCharLogin();

	static CCharLogin * GetInstance()
	{
		if(m_instance == 0)
		{
			m_instance = new CCharLogin();
		}

		return m_instance;
	}

	void DestoryInstance()
	{
		if(m_instance)
		{
			delete m_instance;
			m_instance = 0;
		}
	}

//	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_LS2DB_GETROLEID);
//	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_LS2DB_CREATEROLE);

	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_GM2DB_PLAYERINFO);
	DEF_MSG_SIMPLE_DECLARE_FUN_H(MSG_SIM_GT2GM_SAVE_PLAYERINFO);
	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_GS2DB_SAVE_PLAYERDATA);

	virtual void Handle_Request(Safe_Smart_Ptr<Message> &message);

	virtual void Handle_Message(Safe_Smart_Ptr<Message> &message);

	//玩家基础信息
	void getRoleInfo(int64 playerID,Smart_Ptr<PlayerDBMgr> player,CMysqlInterface& mysqlInterface);

private:
	CCharLogin();

private:
	static CCharLogin * m_instance;
};

#endif /* CHARLOGIN_H_ */
