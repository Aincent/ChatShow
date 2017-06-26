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
#include "SvrConfig.h"
#include "MessageStruct/ServerReturnInt.pb.h"
#include "MessageStruct/ServerReturnIntChar.pb.h"

CharactorLogin * CharactorLogin::m_instance = 0;

CharactorLogin::CharactorLogin()
{
	DEF_MSG_REQUEST_REG_FUN(eGateServer, MSG_REQ_LS2GT_WILLLOGIN);
	DEF_MSG_REQUEST_REG_FUN(eGateServer, MSG_REQ_C2GT_HEARTBEAT);
	DEF_MSG_REQUEST_REG_FUN(eGateServer, MSG_REQ_C2GT_PLAYERINFO);

	DEF_MSG_ACK_REG_FUN(eGateServer, MSG_REQ_GT2GM_PLAYERINFO);

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
	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_C2GT_HEARTBEAT);
	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_C2GT_PLAYERINFO);

	DEF_SWITCH_TRY_DISPATCH_END
}

void CharactorLogin::Handle_Ack(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

	DEF_MSG_ACK_DISPATCH_FUN(MSG_REQ_GT2GM_PLAYERINFO);

	DEF_SWITCH_TRY_DISPATCH_END
}

void CharactorLogin::sendToLoginFail(int64 charid)
{
		ServerReturn::ServerRetInt charlogin;

		charlogin.set_ret(1);

		Safe_Smart_Ptr<CommBaseOut::Message> loginmess  = build_message(MSG_SIM_GT2LS_PLAYERLOGIN, &charlogin, ServerConHandler::GetInstance()->GetLoginServer(), SimpleMessage);
		Message_Facade::Send(loginmess);
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

DEF_MSG_REQUEST_DEFINE_FUN(CharactorLogin, MSG_REQ_C2GT_HEARTBEAT)
{
	LOG_WARNING(FILEINFO, "channel[%d] heart beat [%lld]", message->GetChannelID(), CUtil::GetNowSecond());
	ServerConHandler::GetInstance()->SynchHeartBeat(message->GetChannelID());

	Safe_Smart_Ptr<CommBaseOut::Message> clientRet  = build_message(MSG_REQ_C2GT_HEARTBEAT,message,message->GetChannelID());
	Message_Facade::Send(clientRet);
}

DEF_MSG_REQUEST_DEFINE_FUN(CharactorLogin, MSG_REQ_C2GT_PLAYERINFO)
{
	ServerReturn::ServerRetInt req;
	int len = 0;
	char *str = message->GetBuffer(len);
	int gsChannel = -1;

	req.ParseFromArray(str, len);

	string client_ip = message->GetAddr()->GetIPToString();
	int client_channelID =  message->GetChannelID();

	//移出相应的连接检测链表
	ServerConHandler::GetInstance()->removeConnect(client_channelID);

	if(!ServerConHandler::GetInstance()->UpdateClientInfo(req.ret(), client_channelID, client_ip))
	{
		LOG_WARNING(FILEINFO, "timeout list is't existed[%lld] and close channle[%d]", GET_PLAYER_CHARID(req.ret()), message->GetChannelID());
		Message_Facade::CloseChannel(message->GetChannelID());
		ServerConHandler::GetInstance()->closeClientChannel(message->GetChannelID());

		//通知登录服登录失败
		sendToLoginFail(req.ret());

		return;
	}

	if(ServerConHandler::GetInstance()->GetGSChannelByChannel(client_channelID, gsChannel))
	{
		ServerReturn::ServerRetIntChar toGM;

		toGM.set_retf(req.ret());
		toGM.set_rets(client_ip);

		Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_GT2GM_PLAYERINFO, &toGM, gsChannel, Request);
		messRet->SetAct(new FirstAct(message,req.ret()));
		Message_Facade::Send(messRet);
	}
	else
	{
		//通知登录服登录失败
		sendToLoginFail(req.ret());
		LOG_ERROR(FILEINFO, "Player[charid=%lld] get playerinfo but gameserver not existed", GET_PLAYER_CHARID(req.ret()));
	}
}

DEF_MSG_ACK_DEFINE_FUN(CharactorLogin, MSG_REQ_GT2GM_PLAYERINFO)
{
	if(message->GetErrno() == eReqTimeOut)
	{
		LOG_WARNING(FILEINFO, "gateserver request gameserver player info and ack timeout");
		return;
	}

	int channel = -1;
	int64 charID = message->GetMessageTime();

	if(ServerConHandler::GetInstance()->GetClientChannelByCharID(charID, channel))
	{
		Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_C2GT_PLAYERINFO,message, channel);
		Message_Facade::Send(messRet);
	}
}
