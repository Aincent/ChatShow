/*
 * CharMemCache.cpp
 *
 *  Created on: Jun 29, 2017
 *      Author: root
 */
#include "CharMemCache.h"
#include "Log/Logger.h"
#include "util.h"
#include "SvrConfig.h"

CCharMemCache * CCharMemCache::m_instance = 0;

CacheTimeOut::CacheTimeOut()
{

}
CacheTimeOut::~CacheTimeOut()
{

}

void CacheTimeOut::AddCacheTimeout(int64 charid, DWORD64 time)
{
	GUARD(CSimLock, obj, &m_timeLock);
	m_cacheTime[charid] = time;
}

void CacheTimeOut::AddDelCacheTimeout(int64 charid, DWORD64 time)
{
	GUARD(CSimLock, obj, &m_timeLock);
	m_delCacheTime[charid] = time;
}

void CacheTimeOut::DeleteTimeout(int64 charid)
{
	GUARD(CSimLock, obj, &m_timeLock);
	map<int64, DWORD64>::iterator it = m_cacheTime.find(charid);
	if(it != m_cacheTime.end())
	{
		m_cacheTime.erase(it);
	}
}

void CacheTimeOut::DelCacheTime(int64 charid)
{
	GUARD(CSimLock, obj, &m_timeLock);
	map<int64, DWORD64>::iterator itDel = m_delCacheTime.find(charid);
	if(itDel != m_delCacheTime.end())
	{
		m_delCacheTime.erase(itDel);
	}
}

int CacheTimeOut::svr()
{
	vector<int64> timeout;
	vector<int64> deltimeout;
	while(!m_flag)
	{
		{
			GUARD(CSimLock, obj, &m_timeLock);
			if(m_cacheTime.size() == 0 && m_delCacheTime.size() == 0 )
			{
				obj.UnLock();
				sleep(10);
				continue;
			}

			DWORD64 nowTime = CUtil::GetNowSecond();

			map<int64, DWORD64>::iterator itDel = m_delCacheTime.begin();
			for(; itDel!=m_delCacheTime.end(); )
			{
				if(itDel->second <= nowTime)
				{
						deltimeout.push_back(itDel->first);
						m_cacheTime.erase(itDel->first);
						m_delCacheTime.erase(itDel++);
				}
				else
				{
						++itDel;
				}
			}

			map<int64, DWORD64>::iterator it = m_cacheTime.begin();
			for(; it!=m_cacheTime.end(); ++it)
			{
				if(it->second <= nowTime)
				{
					it->second = nowTime + SAVE_CHARCACHE_TODB;
					timeout.push_back(it->first);
				}
			}

			obj.UnLock();
		}

		CCharMemCache::GetInstance()->SaveCacheToDB(timeout);
		CCharMemCache::GetInstance()->SaveCacheAndDelete(deltimeout);

		timeout.clear();
		deltimeout.clear();
		sleep(10);
	}

	return 0;
}

CCharMemCache::CCharMemCache()
{
	if(m_timeout.Start(1))
	{
		LOG_FATAL(FILEINFO, "Char memory cache thread start error");
	}
}

CCharMemCache::~CCharMemCache()
{
	m_timeout.End();
}

int CCharMemCache::AddNewPlayer(int64 charid, Smart_Ptr<PlayerDBMgr> &player)
{
	GUARD_WRITE(CRWLock, obj, &m_playerLock);
	m_allPlayer[charid] = player;
	obj.UnLock();

	m_timeout.AddCacheTimeout(charid, CUtil::GetNowSecond() + SAVE_CHARCACHE_TODB);

	return 0;
}

bool CCharMemCache::IsPlayerInCache(int64 charid)
{
	GUARD_READ(CRWLock, obj, &m_playerLock);
	map<int64,Smart_Ptr<PlayerDBMgr> >::iterator it = m_allPlayer.find(charid);
	if(it == m_allPlayer.end())
	{
		return false;
	}

	return true;
}

void CCharMemCache::SaveCacheToDB(const std::vector<int64>& chars)
{
	GUARD_READ(CRWLock, obj, &m_playerLock);
	vector<int64>::const_iterator itVec = chars.begin();
	for(; itVec!=chars.end(); ++itVec)
	{
		map<int64,Smart_Ptr<PlayerDBMgr> >::iterator it = m_allPlayer.find(*itVec);
		if(it != m_allPlayer.end())
		{
			if(it->second.Get()==NULL)
				continue;
			int res = it->second->SavePlayerStruct(*itVec);
			if(res==-1)
			{
					LOG_ERROR(FILEINFO, "save player info error,charID=%lld", *itVec);
			}
		}
	}
}

void CCharMemCache::SaveCacheAndDelete(const std::vector<int64>& chars)
{
	GUARD_WRITE(CRWLock, obj, &m_playerLock);
	vector<int64>::const_iterator itVecDel = chars.begin();
	for(; itVecDel!=chars.end(); ++itVecDel)
	{
		map<int64,Smart_Ptr<PlayerDBMgr> >::iterator it = m_allPlayer.find(*itVecDel);
		if(it != m_allPlayer.end())
		{
			if(it->second.Get())
				it->second->SavePlayerStruct(*itVecDel);
			m_allPlayer.erase(it);
		}
	}
}

bool CCharMemCache::SaveCacheToDB(int64 charid)
{
	GUARD_READ(CRWLock, obj, &m_playerLock);
	map<int64,Smart_Ptr<PlayerDBMgr> >::iterator it = m_allPlayer.find(charid);
	if(it != m_allPlayer.end())
	{
		int res = -1;

		res = it->second->SavePlayerStruct(charid);
		if(res == 1)
		{
		}
		else if(res == -1)
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

void CCharMemCache::SaveCacheAndDelete(int64 charid)
{
	GUARD_WRITE(CRWLock, obj, &m_playerLock);
	map<int64,Smart_Ptr<PlayerDBMgr> >::iterator it = m_allPlayer.find(charid);
	if(it != m_allPlayer.end())
	{
		it->second->SavePlayerStruct(charid);
		m_allPlayer.erase(it);
		obj.UnLock();

		m_timeout.DeleteTimeout(charid);
	}
}

void CCharMemCache::SaveToCache(PlayerInfo::SaveTypeInfo *info)
{
	GUARD_WRITE(CRWLock, obj, &m_playerLock);
	map<int64,Smart_Ptr<PlayerDBMgr> >::iterator it = m_allPlayer.find(GET_PLAYER_CHARID(info->id()));
	if(it != m_allPlayer.end())
	{
		for(int i=0; i<eCharStructMax; ++i)
		{
			if((info->type() >> i) & 0x1)
			{
				switch(i)
				{
				case eBaseInfo:
					{
						it->second->AddStruct(i, const_cast<PlayerInfo::BaseInfo *>(&info->bsinfo()));

						break;
					}
				default :
					{
						break;
					}
				}
			}
		}

		if(info->isdel())
		{
			m_timeout.AddDelCacheTimeout(GET_PLAYER_CHARID(info->id()),CUtil::GetNowSecond() + DELETE_CHARCACHE_TIMEOUT);
			it->second->SetFlag(eBaseInfo, true);
			it->second->SavePlayerStruct(GET_PLAYER_CHARID(info->id()));
			obj.UnLock();
		}
	}
}

bool CCharMemCache::GetPlayerInfo(int64 charid, PlayerInfo::PlayerInfo *info)
{
	GUARD_READ(CRWLock, obj, &m_playerLock);
	map<int64,Smart_Ptr<PlayerDBMgr> >::iterator it = m_allPlayer.find(charid);
	if(it != m_allPlayer.end())
	{
		it->second->GetPlayerInfo(info);
	}
	else
	{
		return false;
	}
	obj.UnLock();

	m_timeout.DelCacheTime(charid);
	return true;
}

bool CCharMemCache::GetPlayerInfo(int id, int64 charid, google::protobuf::Message *content)
{
	GUARD_READ(CRWLock, obj, &m_playerLock);
	map<int64,Smart_Ptr<PlayerDBMgr> >::iterator it = m_allPlayer.find(charid);
	if(it != m_allPlayer.end())
	{
		it->second->GetPlayerInfo(id, content);
	}
	else
	{
		return false;
	}
	obj.UnLock();

	m_timeout.DelCacheTime(charid);
	return true;
}

