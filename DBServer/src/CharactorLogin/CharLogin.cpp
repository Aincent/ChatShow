/*
 * CharLogin.cpp
 *
 *  Created on: Jun 29, 2017
 *      Author: root
 */
#include "CharLogin.h"
#include "Log/Logger.h"
#include "Network/Message_Facade.h"
#include "MessageCommonRet.h"
#include "../MessageBuild.h"
#include "MessageStruct/ServerReturnInt.pb.h"
#include "MessageStruct/ServerReturn2Int.pb.h"
#include "MessageStruct/ServerReturn4Int.pb.h"
#include "MessageStruct/ServerReturnChar.pb.h"
#include "../CharMemCache/PlayerDBManager.h"
#include "../CharMemCache/CharMemCache.h"
#include "CharDefine.h"
#include "util.h"
#include <time.h>
#include "SvrConfig.h"
#include "MessageStruct/ServerReturnBool.pb.h"
#include "MessageStruct/CharLogin/PlayerInfo.pb.h"

CCharLogin * CCharLogin::m_instance = 0;

CCharLogin::CCharLogin()
{
//	DEF_MSG_REQUEST_REG_FUN(eDBServer, MSG_REQ_LS2DB_GETROLEID);
//	DEF_MSG_REQUEST_REG_FUN(eDBServer, MSG_REQ_LS2DB_CREATEROLE);
	DEF_MSG_REQUEST_REG_FUN(eDBServer, MSG_REQ_GM2DB_PLAYERINFO);
	DEF_MSG_SIMPLE_REG_FUN(eDBServer, MSG_SIM_GT2GM_SAVE_PLAYERINFO);
	DEF_MSG_REQUEST_REG_FUN(eDBServer, MSG_REQ_GS2DB_SAVE_PLAYERDATA);

//	InitCharTemplate();
}

CCharLogin::~CCharLogin()
{

}

void CCharLogin::Handle_Request(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN

//	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_LS2DB_GETROLEID);
//	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_LS2DB_CREATEROLE);
	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_GM2DB_PLAYERINFO);
	DEF_MSG_REQUEST_DISPATCH_FUN(MSG_REQ_GS2DB_SAVE_PLAYERDATA);

	DEF_SWITCH_TRY_DISPATCH_END
}

void CCharLogin::Handle_Message(Safe_Smart_Ptr<Message> &message)
{
	DEF_SWITCH_TRY_DISPATCH_BEGIN
	DEF_MSG_SIMPLE_DISPATCH_FUN(MSG_SIM_GT2GM_SAVE_PLAYERINFO);
	DEF_SWITCH_TRY_DISPATCH_END
}

DEF_MSG_REQUEST_DEFINE_FUN(CCharLogin, MSG_REQ_GM2DB_PLAYERINFO)
{
	LOG_DEBUG(FILEINFO, "gameserver request dbserver player info");

#ifdef DEBUG
	int64 betime = CUtil::GetNowSecond();
#endif

	ServerReturn::ServerRetInt req;
	Smart_Ptr<PlayerInfo::PlayerInfo> content = new PlayerInfo::PlayerInfo();
	char * str;
	int len = 0;

	str = message->GetBuffer(len);
	req.ParseFromArray(str, len);

	if(!CCharMemCache::GetInstance()->IsPlayerInCache(GET_PLAYER_CHARID(req.ret())))
	{
		Smart_Ptr<PlayerDBMgr> tempPlayer = new PlayerDBMgr();
		try
		{
			CMysqlInterface tempMysqlInterface;

			getRoleInfo(req.ret(),tempPlayer,tempMysqlInterface);

			CCharMemCache::GetInstance()->AddNewPlayer(GET_PLAYER_CHARID(req.ret()), tempPlayer);
		}
		catch(exception &e)
		{
			LOG_ERROR(FILEINFO, "[messageid = %d]Get player cache error , errormsg [%s]", MSG_REQ_GM2DB_PLAYERINFO, e.what());

			return;
		}
		catch(...)
		{
			LOG_ERROR(FILEINFO, "[messageid = %d]Get player cache unknown error", MSG_REQ_GM2DB_PLAYERINFO);

			return;
		}
	}

	CCharMemCache::GetInstance()->GetPlayerInfo(req.ret(),content.Get());
	if(!content)
	{
		LOG_ERROR(FILEINFO, "[messageid = %d]Get player cache error", MSG_REQ_GM2DB_PLAYERINFO);

		return;
	}

	if(content->ByteSize() >= 32 * 1024)
	{
		LOG_FATAL(FILEINFO, "[messageid = %d]Get player cache but player info size > 32 * 1024", MSG_REQ_GM2DB_PLAYERINFO);

		return;
	}

	Safe_Smart_Ptr<CommBaseOut::Message> messRet  = build_message(MSG_REQ_GM2DB_PLAYERINFO, message, content.Get(), Ack);
	Message_Facade::Send(messRet);


#ifdef DEBUG
	int64 endTime = CUtil::GetNowSecond();

	if(endTime - betime > 10 || (GET_PLAYER_CHARID(req.ret())) == 37143)
	{
		LOG_FATAL(FILEINFO, "%s and too more time[%lld] player[%lld]", __FUNCTION__, endTime - betime,GET_PLAYER_CHARID(req.ret()));
	}
#endif
}

//玩家基础信息
void CCharLogin::getRoleInfo(int64 playerID,Smart_Ptr<PlayerDBMgr> player,CMysqlInterface& mysqlInterface)
{
	try
	{
		mysqlInterface.Select("select * from RoleInfo where CharID=%lld",GET_PLAYER_CHARID(playerID));
		if(!mysqlInterface.IsHaveRecord())
			return ;

		Smart_Ptr<PlayerInfo::BaseInfo> baseInfo = new PlayerInfo::BaseInfo();

		baseInfo->set_charid(mysqlInterface.GetInt64Name("CharID"));
		baseInfo->set_charname(mysqlInterface.GetStringName("CharName"));


		player->AddStruct(eBaseInfo, baseInfo);
	}
	catch(exception &e)
	{
		LOG_ERROR(FILEINFO, "getRoleInfo error,errormsg [%s]",e.what());
	}
	catch(...)
	{
		LOG_ERROR(FILEINFO, "getRoleInfo unknown error");
	}

	return ;
}

DEF_MSG_SIMPLE_DEFINE_FUN(CCharLogin, MSG_SIM_GT2GM_SAVE_PLAYERINFO)
{
	//LOG_DEBUG(FILEINFO, "[messageid = %d] save player info", MSG_SIM_GT2GM_SAVE_PLAYERINFO);

	PlayerInfo::SaveTypeInfo meContent;
	char *str;
	int len = 0;

	str = message->GetBuffer(len);
	meContent.ParseFromArray(str, len);
	CCharMemCache::GetInstance()->SaveToCache(&meContent);
}

DEF_MSG_REQUEST_DEFINE_FUN(CCharLogin, MSG_REQ_GS2DB_SAVE_PLAYERDATA)
{
	PlayerInfo::SaveTypeInfo meContent;
	char *str;
	int len = 0;
	str = message->GetBuffer(len);
	meContent.ParseFromArray(str, len);
	CCharMemCache::GetInstance()->SaveToCache(&meContent);

	ServerReturn::ServerRetInt ret;
	ret.set_ret(meContent.id());

	Safe_Smart_Ptr<Message> messRet = build_message(MSG_REQ_GS2DB_SAVE_PLAYERDATA,message, &ret);
	Message_Facade::Send(messRet);
}
