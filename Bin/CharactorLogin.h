/*
 * CharactorLogin.h
 *
 *  Created on: 2013��9��24��
 *      Author: helh
 */

#ifndef CHARACTORLOGIN_H_
#define CHARACTORLOGIN_H_

#include "Network/MessageHandler.h"
#include "MsgDefineMacro.h"
#include "ServerMsgDefine.h"

using namespace CommBaseOut;

class CharactorLogin : public Message_Handler, public Request_Handler, public Ack_Handler
{
public:
	~CharactorLogin();

	static CharactorLogin *GetInstance()
	{
		if(m_instance == 0)
		{
			m_instance = new CharactorLogin();
		}

		return m_instance;
	}
	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_C2GT_CLIENTIN);
	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_GM2GT_CHANGEMAP);
	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_C2GT_PLAYERINFO);
	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_LG2GT_PLAYER_EXIT);
	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_SIM_LS2GT_WILLLOGIN);
	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_C2GT_CHANGEMAP);
	DEF_MSG_REQUEST_DECLARE_FUN_H(MSG_REQ_GM2GT_PLAYER_ONLINE);
	DEF_MSG_ACK_DECLARE_FUN_H(MSG_SIM_GT2WS_PLAYEREXIT);
	DEF_MSG_ACK_DECLARE_FUN_H(MSG_REQ_GT2GM_PLAYERINFO);
	DEF_MSG_ACK_DECLARE_FUN_H(MSG_REQ_GT2GM_CHANGEMAP);
	DEF_MSG_ACK_DECLARE_FUN_H(MSG_REQ_GT2GM_CLIENTIN);
	DEF_MSG_ACK_DECLARE_FUN_H(MSG_REQ_GT2GM_PLAYER_ONLINE);

	virtual void Handle_Message(Safe_Smart_Ptr<Message> &message);
	virtual void Handle_Request(Safe_Smart_Ptr<Message> &message);
	virtual void Handle_Ack(Safe_Smart_Ptr<Message> &message);

	void DestroyInstance()
	{
		if(m_instance)
		{
			delete m_instance;
			m_instance = 0;
		}
	}

	//通知登录服此次登录失败
	void sendToLoginFail(int64 charid);

private:
	CharactorLogin();

private:
	static CharactorLogin * m_instance;
};


#endif /* CHARACTORLOGIN_H_ */
