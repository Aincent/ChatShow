/*
 * CharMemCache.h
 *
 *  Created on: Jun 29, 2017
 *      Author: root
 */

#ifndef CHARMEMCACHE_H_
#define CHARMEMCACHE_H_

#include <map>
#include "Thread/Task.h"
#include "define.h"
#include "PlayerDBManager.h"
#include "Common.h"
#include "MessageStruct/CharLogin/PlayerInfo.pb.h"

using namespace std;
using namespace CommBaseOut;

class CacheTimeOut : public Task
{
public:
	CacheTimeOut();
	virtual ~CacheTimeOut();

	void AddCacheTimeout(int64 charid, DWORD64 time);

	void AddDelCacheTimeout(int64 charid, DWORD64 time);

	void DeleteTimeout(int64 charid);

	void DelCacheTime(int64 charid);

private:

	virtual int svr();

private:
	map<int64, DWORD64> m_cacheTime;
	map<int64, DWORD64> m_delCacheTime;
	CSimLock m_timeLock;
};

class CCharMemCache
{
public:
	~CCharMemCache();

	static CCharMemCache* GetInstance()
	{
		if(m_instance == 0)
		{
			m_instance = new CCharMemCache();
		}

		return m_instance;
	}

	void DestroyInstance()
	{
		if(m_instance)
		{
			delete m_instance;
			m_instance = 0 ;
		}
	}

	int AddNewPlayer(int64 charid, Smart_Ptr<PlayerDBMgr> &player);

	bool IsPlayerInCache(int64 charid);

	bool SaveCacheToDB(int64 charid);
	void SaveCacheAndDelete(int64 charid);

	//批量保存删除
	void SaveCacheToDB(const std::vector<int64>& chars);
	void SaveCacheAndDelete(const std::vector<int64>& chars);

	void SaveToCache(PlayerInfo::SaveTypeInfo *info);

	bool GetPlayerInfo(int64 charid, PlayerInfo::PlayerInfo *info);
	bool GetPlayerInfo(int id, int64 charid, google::protobuf::Message *content);

private:
	CCharMemCache();

private:
	static CCharMemCache * m_instance;

	CacheTimeOut m_timeout;
	map<int64,Smart_Ptr<PlayerDBMgr> > m_allPlayer;
	CRWLock m_playerLock;
};

#endif /* CHARMEMCACHE_H_ */
