/*
 * PlayerDBManager.cpp
 *
 *  Created on: Jun 29, 2017
 *      Author: root
 */
#include "PlayerDBManager.h"
#include "util.h"
#include "SvrConfig.h"
#include "SqlInterface/MysqlInterface.h"
#include "Log/Logger.h"
#include "../ServerManager.h"

CStructTemplate::CStructTemplate(int type):m_type(type),m_flag(false)
{

}

CStructTemplate::~CStructTemplate()
{

}

int CStructTemplate::SavePlayerBaseInfo(PlayerInfo::BaseInfo *info)
{
#ifdef DEBUG
	int64 betime = CUtil::GetNowSecond();
#endif
	try
	{
		CMysqlInterface con;
		int64 teamId = 0;

		int64 nowTime = CUtil::GetNowSecond();

		con.Execute("update RoleInfo set CharName='%s',");


		con.GetNextRow();
	}
	catch(exception &e)
	{
		LOG_ERROR(FILEINFO, "timeout Save player baseinfo cache error , errormsg [%s]", e.what());

		return -1;
	}
	catch(...)
	{
		LOG_ERROR(FILEINFO, "timeout Save player baseinfo cache unknown error");

		return -1;
	}
#ifdef DEBUG
	int64 endTime = CUtil::GetNowSecond();

	if(endTime - betime > 10 )
	{
		LOG_FATAL(FILEINFO, "%s and too more time[%lld] player[%lld]", __FUNCTION__, endTime - betime, GET_PLAYER_CHARID(info->charid()));
	}
#endif
	return 0;
}


int CStructTemplate::Save(int64 charid)
{
	switch(m_type)
	{
	case eBaseInfo:
		{
			GUARD(CSimLock, obj, &m_structLock);
			if(!m_flag)
			{
				return 1;
			}

			PlayerInfo::BaseInfo *info = static_cast<PlayerInfo::BaseInfo *>(m_content.Get());
			if(info != 0)
			{
				int res = SavePlayerBaseInfo(info);
				if(res == 0)
				{
					m_flag = false;
				}

				return res;
			}
			else
			{
				LOG_ERROR(FILEINFO, "save player[%lld] info[type   %d] is null", charid, m_type);
			}

			break;
		}
	default:
		{
			LOG_ERROR(FILEINFO, "Save player struct type[m_type=%d] error", m_type);

			break;
		}
	}

	return -1;
}

void CStructTemplate::AddContent(Smart_Ptr<google::protobuf::Message> &content)
{
	GUARD(CSimLock, obj, &m_structLock);
	m_content = content;
}

void CStructTemplate::AddContent(google::protobuf::Message *content)
{
	GUARD(CSimLock, obj, &m_structLock);
	m_content->Clear();
	m_content->CopyFrom(*content);
	m_flag = true;
}


PlayerDBMgr::PlayerDBMgr():m_beTime(0)
{
	for(int i=0; i<eCharStructMax; ++i)
	{
		m_pStruct[i] = new CStructTemplate(i);
	}
}

PlayerDBMgr::~PlayerDBMgr()
{

}

void PlayerDBMgr::SetFlag(int id, bool flag)
{
	if(id >= eCharStructMax)
		return;

	m_pStruct[id]->SetFlag(flag);
}

int PlayerDBMgr::SavePlayerStruct(int64 charid)
{
	int res = 0;
	int count = 0;

	for(int i=0; i<eCharStructMax; ++i)
	{
		if(!m_pStruct[i])
		{
			LOG_ERROR(FILEINFO, "save player[%d] but struct[%d] is null", charid, i);
			continue;
		}

		res = m_pStruct[i]->Save(charid);

		if(res == -1)
		{
			count++;
		}
	}

	if(count > 0)
	{
		return -1;
	}

	return res;
}

void PlayerDBMgr::AddStruct(int type, Smart_Ptr<google::protobuf::Message> content)
{
	m_pStruct[type]->AddContent(content);
}

void PlayerDBMgr::AddStruct(int type, google::protobuf::Message *content)
{
	m_pStruct[type]->AddContent(content);
}


bool PlayerDBMgr::GetPlayerInfo(PlayerInfo::PlayerInfo *info)
{
	PlayerInfo::BaseInfo * newBsinfo = info->mutable_bsinfo();
	m_pStruct[eBaseInfo]->GetContent(newBsinfo);

	int64 charID = CREATE_CHARID_GS(ServerConHandler::GetInstance()->GetSvrID(),newBsinfo->charid());
	newBsinfo->set_charid(charID);

	return true;
}

bool PlayerDBMgr::GetPlayerInfo(int id, google::protobuf::Message *content)
{
	m_pStruct[id]->GetContent(content);

	return true;
}

Smart_Ptr<google::protobuf::Message> PlayerDBMgr::getProtobuf(ECharBaseStruct type)
{
	if(type < eBaseInfo || type >= eCharStructMax)
	{
		return Smart_Ptr<google::protobuf::Message>();
	}

	return m_pStruct[type]->getContentNoLock();
}
