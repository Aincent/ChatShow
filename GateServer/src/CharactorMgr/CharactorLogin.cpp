/*
 * CharactorLogin.cpp
 *
 *  Created on: Jun 17, 2017
 *      Author: root
 */
#include "CharactorLogin.h"
#include "../ServerManager.h"
#include "Log/Logger.h"
#include "define.h"
#include "../MessageBuild.h"
#include "../MsgCommonClass.h"
#include "../SvrConfig.h"

CharactorLogin * CharactorLogin::m_instance = 0;

CharactorLogin::CharactorLogin()
{
	DEF_MSG_REQUEST_REG_FUN(eGateServer, MSG_REQ_LS2GT_WILLLOGIN);
}

CharactorLogin::~CharactorLogin()
{

}

void CharactorLogin::Handle_Message(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

	DEF_SWITCH_TRY_DISPATCH_END
}

void CharactorLogin::Handle_Request(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_LS2GT_WILLLOGIN);

	DEF_SWITCH_TRY_DISPATCH_END
}

void CharactorLogin::Handle_Ack(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

//	DEF_MSG_ACK_DISPATCH_FUN(MSG_REQ_GT2WS_MAPLINEINFO);

	DEF_SWITCH_TRY_DISPATCH_END
}

DEF_MSG_REQUEST_DEFINE_FUN(CharactorLogin, MSG_REQ_LS2GT_WILLLOGIN)
{
	int len = 0;
	int off = 0;
	char *content = message->GetBuffer(len);
	int64 charid = CUtil::ntohll(*((uint64_t*)content));
	int namelen = (ntohl(*((int*)(content + 8))));
	char namebuff[33] = {0};
	memcpy(namebuff,content+12,namelen);

	int code = 0;

	if(!ServerConHandler::GetInstance()->AddClientTimeOut(charid, message))
	{
		code = 1;
	}

	int nPort  = ServerConHandler::GetInstance()->GetPort();
	int nLocalID = ServerConHandler::GetInstance()->GetLocalID();
	char buf[128] = {0};
	memcpy(buf + off,&code,4);
	off += 4;
	memcpy(buf + off,&nPort,4);
	off += 4;
	memcpy(buf + off,&nLocalID,2);
	off += 2;
	len = ServerConHandler::GetInstance()->GetIP().size();
	memcpy(buf+off,&len,4);
	off += 4;
	memcpy(buf+off,ServerConHandler::GetInstance()->GetIP().c_str(),len);
	off += len;
	memcpy(buf + off,&namelen,4);
	off += 4;
	memcpy(buf+off,namebuff,namelen);
	off += namelen;

	Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_LS2GT_WILLLOGIN, buf,off, message->GetChannelID());
	messRet->SetMessageTime(message->GetMessageTime());
	Message_Facade::Send(messRet);
}

