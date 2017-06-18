/*
 * CharactorLogin.cpp
 *
 *  Created on: 2013��9��24��
 *      Author: helh
 */

#include "CharactorLogin.h"
#include "../ServerManager.h"
#include "Log/Logger.h"
#include "MessageStruct/CharLogin/LoginGatePacket.pb.h"
#include "MessageStruct/CharLogin/RetCharLoginInfo.pb.h"
#include "MessageStruct/ServerReturnChar.pb.h"
#include "MessageStruct/ServerReturnInt.pb.h"
#include "MessageStruct/ServerReturnIntChar.pb.h"
#include "MessageStruct/ServerReturn3Int.pb.h"
#include "MessageStruct/ServerReturn4Int.pb.h"
#include "MessageStruct/ServerReturn2Int.pb.h"
#include "MessageStruct/ServerReturn5Int.pb.h"
#include "define.h"
#include "MessageStruct/CharLogin/CharBaseInfo.pb.h"
#include "MessageStruct/Map/ChangeMap.pb.h"
#include "MessageStruct/CharLogin/PlayerInfo2Path.pb.h"
#include "MessageStruct/CharLogin/PlayerInfo.pb.h"
#include "../MessageBuild.h"
#include "../MsgCommonClass.h"
#include "SvrConfig.h"
#include "CharDefine.h"
#include "./MapLoader/MapConfig.h"
#include "MessageStruct/CharLogin/CharBaseInfo.pb.h"

CharactorLogin * CharactorLogin::m_instance = 0;

CharactorLogin::CharactorLogin()
{
	DEF_MSG_REQUEST_REG_FUN(eGateServer, MSG_SIM_LS2GT_WILLLOGIN);
	DEF_MSG_REQUEST_REG_FUN(eGateServer, MSG_REQ_C2GT_CLIENTIN);
	DEF_MSG_REQUEST_REG_FUN(eGateServer, MSG_REQ_GM2GT_CHANGEMAP);
	DEF_MSG_REQUEST_REG_FUN(eGateServer, MSG_REQ_GM2GT_PLAYER_ONLINE);
	DEF_MSG_REQUEST_REG_FUN(eGateServer, MSG_REQ_C2GT_PLAYERINFO);
	DEF_MSG_REQUEST_REG_FUN(eGateServer, MSG_REQ_LG2GT_PLAYER_EXIT);
	DEF_MSG_REQUEST_REG_FUN(eGateServer, MSG_REQ_C2GT_CHANGEMAP);

	DEF_MSG_ACK_REG_FUN(eGateServer, MSG_SIM_GT2WS_PLAYEREXIT);
	DEF_MSG_ACK_REG_FUN(eGateServer, MSG_REQ_GT2GM_PLAYERINFO);
	DEF_MSG_ACK_REG_FUN(eGateServer, MSG_REQ_GT2GM_CHANGEMAP);
	DEF_MSG_ACK_REG_FUN(eGateServer, MSG_REQ_GT2GM_CLIENTIN);
	DEF_MSG_ACK_REG_FUN(eGateServer, MSG_REQ_GT2GM_PLAYER_ONLINE);
}

CharactorLogin::~CharactorLogin()
{

}

void CharactorLogin::sendToLoginFail(int64 charid)
{
		ServerReturn::ServerFourInt charlogin;

		charlogin.set_retf(1);

		Safe_Smart_Ptr<CommBaseOut::Message> loginmess  = build_message(MSG_SIM_GT2LS_PLAYERLOGIN, &charlogin, ServerConHandler::GetInstance()->GetLoginServer(), SimpleMessage);
		Message_Facade::Send(loginmess);
}

void CharactorLogin::Handle_Message(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

	DEF_SWITCH_TRY_DISPATCH_END
}

void CharactorLogin::Handle_Request(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_C2GT_CLIENTIN);
	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_C2GT_PLAYERINFO);
	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_LG2GT_PLAYER_EXIT);
	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_SIM_LS2GT_WILLLOGIN);

	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_C2GT_CHANGEMAP);
	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_GM2GT_CHANGEMAP);
	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_GM2GT_PLAYER_ONLINE);

	DEF_SWITCH_TRY_DISPATCH_END
}

void CharactorLogin::Handle_Ack(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

	DEF_MSG_ACK_DISPATCH_FUN(MSG_REQ_GT2GM_PLAYERINFO);
	DEF_MSG_ACK_DISPATCH_FUN(MSG_REQ_GT2GM_CHANGEMAP);
	DEF_MSG_ACK_DISPATCH_FUN(MSG_REQ_GT2GM_CLIENTIN);
	DEF_MSG_ACK_DISPATCH_FUN(MSG_SIM_GT2WS_PLAYEREXIT);
	DEF_MSG_ACK_DISPATCH_FUN(MSG_REQ_GT2GM_PLAYER_ONLINE);

//	DEF_MSG_ACK_DISPATCH_FUN(MSG_REQ_GT2WS_MAPLINEINFO);

	DEF_SWITCH_TRY_DISPATCH_END
}

DEF_MSG_REQUEST_DEFINE_FUN(CharactorLogin, MSG_REQ_GM2GT_CHANGEMAP)
{
	ServerReturn::ServerThreeInt ret;
	int len = 0;
	char *content = message->GetBuffer(len);
	int64 sceneid = -1;
	int xpos = 0;
	int ypos = 0;

	ret.ParseFromArray(content, len);

	switch(ret.rett()&0xffffffff)
	{
		case eMapType:					//根据地图ID传送
		{
			short int retMap = ret.rets();
			sceneid = ServerConHandler::GetInstance()->GetSceneIDByMapID(retMap);
			if(sceneid > 0)
			{
				xpos = (ret.rett() >> 48) & 0xffff;
				ypos = (ret.rett() >> 32) & 0xffff;
			}
			break;
		}

		case	 eSceneType:		//根据场景ID传送
		case eBePointType: //帮派战传送
		{
			//优先传送原来进入的点，如果找不到的话，自动找相同地图分配
			int64 tMapID = ret.rets();
			if(ServerConHandler::GetInstance()->IsHaveSceneID(tMapID))
			{
					sceneid = tMapID;
			}
			else
			{
					MapStaticInfo * sInfo = CMapConfig::GetInstance()->GetMapStaticInfoByMapID(GET_MAP_ID(tMapID));
					if(sInfo == NULL)
							return;
					sceneid = ServerConHandler::GetInstance()->GetSceneIDByMapID(GET_MAP_ID(tMapID));
			}

			if(sceneid > 0)
			{
					xpos = (ret.rett() >> 48) & 0xffff;
					ypos = (ret.rett() >> 32) & 0xffff;
			}
			break;
		}

		case eStartPoint:	//根据传送阵ID传送
		{
			MapStartPoint * retMap = CMapConfig::GetInstance()->GetStartPoint(ret.rets());
			sceneid = ServerConHandler::GetInstance()->GetSceneIDByMapID(retMap->tMapID);
			if(sceneid > 0)
			{
				xpos = retMap->tXPos;
				ypos = retMap->tYPos;
			}
			break;
		}

		case eChangeMapLine:
		{
			//验证有没有该场景
			if(!ServerConHandler::GetInstance()->IsHaveSceneID(ret.rets()))
			{
					LOG_ERROR(FILEINFO, "change line:cannot find sceneID");
					return;
			}
			sceneid = ret.rets();
			xpos = (ret.rett() >> 48) & 0xffff;
			ypos = (ret.rett() >> 32) & 0xffff;
			break;
		}

		default:
		{
			return;
		}
	}

	if(sceneid <= 0)
	{
		return;
	}

	ServerReturn::ServerThreeInt toGM;
	toGM.set_retf(ret.retf());
	toGM.set_rets(sceneid);
	toGM.set_rett(((int64)xpos << 32) | ypos);

	Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_GM2GT_CHANGEMAP, message, &toGM,Ack,15);
	Message_Facade::Send(messRet);
}

DEF_MSG_REQUEST_DEFINE_FUN(CharactorLogin, MSG_REQ_GM2GT_PLAYER_ONLINE)
{
	ServerReturn::ServerFiveInt  ret;
	int len = 0;
	char *content = message->GetBuffer(len);
	Safe_Smart_Ptr<SvrItem> gaSvr;

	ret.ParseFromArray(content, len);

	if(!ServerConHandler::GetInstance()->GetGameServerInfo(ret.rets(), gaSvr))
	{
		LOG_ERROR(FILEINFO, "player online after changing map but error");

		return ;
	}

	//检测宕机条件
	if(gaSvr.Get()==NULL)
	{
		return;
	}

	ret.set_retf(ret.retf());

	Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_GT2GM_PLAYER_ONLINE, &ret, gaSvr.Get(), Request);
	messRet->SetAct(new requestAct(message));
	Message_Facade::Send(messRet);
}

DEF_MSG_REQUEST_DEFINE_FUN(CharactorLogin, MSG_REQ_C2GT_CHANGEMAP)
{
	ServerReturn::ServerRetInt ret;
	int len = 0;
	char *content = message->GetBuffer(len);
	int64 charid = -1;
	int gsChannel = -1;

	ret.ParseFromArray(content, len);

	//属于副本则放它过去；1为唯一地图，2为静态地图
	int instanceType = (ret.ret() >> 40) & 0xfff;
	if(!CMapConfig::GetInstance()->IsHaveStartPoint(ret.ret()) && (instanceType <= 2 && instanceType>= 1))
	{
		LOG_ERROR(FILEINFO, "change map but start point is null");
		return;
	}

	if(ServerConHandler::GetInstance()->GetGSChannelAndCharIDByChannel(message->GetChannelID(), gsChannel, charid))
	{
		ServerReturn::ServerDoubleInt meContent;

		meContent.set_retf(charid);
		meContent.set_rets(ret.ret());

		Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_GT2GM_CHANGEMAP, &meContent, gsChannel, Request);
		messRet->SetAct(new requestAct(message));
		Message_Facade::Send(messRet);
	}
}

DEF_MSG_REQUEST_DEFINE_FUN(CharactorLogin, MSG_SIM_LS2GT_WILLLOGIN)
{
	CharLogin::RetCharLoginInfo ret;
	int len = 0;
	char *content = message->GetBuffer(len);
	ret.ParseFromArray(content, len);

	ServerReturn::ServerThreeIntTwoChar retLogin;

	retLogin.set_code(0);

	if(!ServerConHandler::GetInstance()->AddClientTimeOut(ret.mapid(), ret.charid(), message))
	{
		retLogin.set_code(1);
	}

	retLogin.set_rets(ServerConHandler::GetInstance()->GetPort());
	retLogin.set_rett(ServerConHandler::GetInstance()->GetLocalID());
	retLogin.set_retf(ServerConHandler::GetInstance()->GetIP());
	retLogin.set_retfi(ret.openid());

	Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_SIM_LS2GT_WILLLOGIN, message, &retLogin);
	messRet->SetMessageTime(message->GetMessageTime());
	Message_Facade::Send(messRet);
}

DEF_MSG_ACK_DEFINE_FUN(CharactorLogin, MSG_SIM_GT2WS_PLAYEREXIT)
{
	LOG_WARNING(FILEINFO,"ServerConHandler::PlayerExit(const int64 &charid, Safe_Smart_Ptr<Message> message).................................. %lld", GET_PLAYER_CHARID(static_cast<commonAct *>(message->GetRequest()->GetAct().Get())->mesDataf));
	ServerConHandler::GetInstance()->PlayerExit(static_cast<commonAct *>(message->GetRequest()->GetAct().Get())->mesDataf, message);
}

DEF_MSG_REQUEST_DEFINE_FUN(CharactorLogin, MSG_REQ_C2GT_CLIENTIN)
{
	int64 charid = -1;
	int gsChannel = -1;
	CharLogin::SyncMap syncMap;

	if(ServerConHandler::GetInstance()->GetGSChannelAndCharIDByChannelEx(message->GetChannelID(), gsChannel, charid))
	{
		LOG_DEBUG(FILEINFO, "client[%lld] request message[%d] client in and gt request gm", GET_PLAYER_CHARID(charid), message->GetMessageID());

		Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_GT2GM_CLIENTIN,message, gsChannel, Request,30);
		messRet->SetMessageTime(charid);
		messRet->SetAct(new requestAct(message));
		Message_Facade::Send(messRet);
	}
	else
	{
		LOG_DEBUG(FILEINFO, "client[%lld] request message[%d] client in and ack to client", GET_PLAYER_CHARID(charid), message->GetMessageID());

		Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_C2GT_CLIENTIN, message,&syncMap);
		Message_Facade::Send(messRet);
	}
}

DEF_MSG_REQUEST_DEFINE_FUN(CharactorLogin, MSG_REQ_LG2GT_PLAYER_EXIT)
{
	LOG_DEBUG(FILEINFO, "Loginserver request kick player[%lld]", GET_PLAYER_CHARID(message->GetMessageTime()));

	int channel = -1;
	int64 charID = message->GetMessageTime();

	if(ServerConHandler::GetInstance()->GetClientChannelByCharID(charID, channel))
	{
		ServerReturn::ServerRetInt con;

		con.set_ret(eTypeOtherClient);

		Safe_Smart_Ptr<CommBaseOut::Message> clientRet  = build_message(MSG_SIM_GT2C_PLAYER_EXIT, &con, channel, SimpleMessage);
		Message_Facade::Send(clientRet);
		LOG_WARNING(FILEINFO,"ServerConHandler::PlayerExit(const int64 &charid, bool flag, Safe_Smart_Ptr<Message> message).................................. %lld", GET_PLAYER_CHARID(charID));
		ServerConHandler::GetInstance()->PlayerExit(charID, true, message);
	}
	else
	{
		ServerReturn::ServerRetInt ret;
		ret.set_ret(charID);

		Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_LG2GT_PLAYER_EXIT, message, &ret);
		Message_Facade::Send(messRet);
	}
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

DEF_MSG_ACK_DEFINE_FUN(CharactorLogin, MSG_REQ_GT2GM_PLAYER_ONLINE)
{
	if(message->GetErrno() == eReqTimeOut)
	{
		LOG_WARNING(FILEINFO, "gateserver request gameserver player change map and online but ack timeout");

		return;
	}

	LOG_DEBUG(FILEINFO,"gateserver request gameserver player change map and online but ack ");

	ChangeMap::ChangeMapInfo meContent;
	char *con;
	int len = 0;

	con = message->GetBuffer(len);
	meContent.ParseFromArray(con, len);
	ServerConHandler::GetInstance()->UpdateSceneidByChar(meContent.charid(), meContent.mapid(), message->GetRemoteID());

	Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(static_cast<requestAct *>(act.Get())->mesReq->GetMessageID(), static_cast<requestAct *>(act.Get())->mesReq, &meContent);
	Message_Facade::Send(messRet);
}

DEF_MSG_ACK_DEFINE_FUN(CharactorLogin, MSG_REQ_GT2GM_CHANGEMAP)
{
	if(message->GetErrno() == eReqTimeOut)
	{
		LOG_WARNING(FILEINFO, "client request gateserver player change map and ack timeout");
		return;
	}

	LOG_DEBUG(FILEINFO,"gateserver request gateserver player change map and ack ");
	ChangeMap::ChangeMapInfo meContent;
	int len = 0;
	char * con = message->GetBuffer(len);
	meContent.ParseFromArray(con, len);

	if(meContent.has_ip())
	{
		ClientLogin::LoginGateInfo retGateInfo;

		retGateInfo.set_charid(meContent.charid());
		retGateInfo.set_ip(meContent.ip());
		retGateInfo.set_port(meContent.port());
		retGateInfo.set_gateid(meContent.gateid());

		int channel = -1;

		if(ServerConHandler::GetInstance()->GetClientChannelByCharID(meContent.charid(), channel))
		{
			Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_SIM_GT2C_CHANGE_SERVER, &retGateInfo, channel, SimpleMessage);
			Message_Facade::Send(messRet);
		}
	}
	else
	{
		int64 packRet = 0;
		ServerReturn::ServerThreeInt toClient;

		if(meContent.mapid() < 0)
		{
			packRet = (meContent.mapid() << 32);
		}
		else
		{
			packRet = ((int64)GET_MAP_ID(meContent.mapid())) << 32;
			packRet |= meContent.pos();
		}

		toClient.set_retf(packRet);
		toClient.set_rets(meContent.key());
		toClient.set_rett(meContent.pktype());

		Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_C2GT_CHANGEMAP, static_cast<requestAct *>(act.Get())->mesReq, &toClient);
		Message_Facade::Send(messRet);
	}
}

DEF_MSG_ACK_DEFINE_FUN(CharactorLogin, MSG_REQ_GT2GM_CLIENTIN)
{
	if(message->GetErrno() == eReqTimeOut)
	{
		LOG_ERROR(FILEINFO, "gateserver request gameserver client in and ack timeout");
		return;
	}

	CharLogin::SyncMap syncMap;
	int len = 0;
	char *str = message->GetBuffer(len);
	syncMap.ParseFromArray(str, len);

	if(syncMap.charid() > 0)
	{
		LOG_WARNING(FILEINFO,"ServerConHandler::PlayerLogin(const int64 &charid, const int64 & mapid)................channel[%d].................. %lld",static_cast<requestAct *>(act.Get())->mesReq->GetChannelID(), GET_PLAYER_CHARID(syncMap.charid()));

		if(!ServerConHandler::GetInstance()->PlayerLogin(syncMap.charid(), syncMap.mapid()))
		{
			Safe_Smart_Ptr<CommBaseOut::Message> mesReq;
			ServerReturn::ServerDoubleInt gamecharid;

			gamecharid.set_retf(syncMap.charid());
			gamecharid.set_rets(1);

			Safe_Smart_Ptr<CommBaseOut::Message> gamemess = build_message(MSG_REQ_GT2GM_PLAYEREXIT, message, &gamecharid, Request);
			gamemess->SetAct(new FirstAct(mesReq, syncMap.charid()));
			Message_Facade::Send(gamemess);

			sendToLoginFail(syncMap.charid());

			return;
		}

		ServerReturn::ServerFourInt charlogin;

		charlogin.set_retf(0);
		charlogin.set_rets(syncMap.charid());
		charlogin.set_rett(syncMap.mapid());
		charlogin.set_retfo((ServerConHandler::GetInstance()->GetLocalType() << 16) | ServerConHandler::GetInstance()->GetLocalID());

		Safe_Smart_Ptr<CommBaseOut::Message> loginmess  = build_message(MSG_SIM_GT2LS_PLAYERLOGIN, &charlogin, ServerConHandler::GetInstance()->GetLoginServer(), SimpleMessage);
		Message_Facade::Send(loginmess);
	}

	Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_C2GT_CLIENTIN, static_cast<requestAct *>(act.Get())->mesReq,&syncMap);
	Message_Facade::Send(messRet);
}


