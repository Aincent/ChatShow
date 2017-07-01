/*
 * PlayerDBManager.h
 *
 *  Created on: Jun 29, 2017
 *      Author: root
 */

#ifndef PLAYERDBMANAGER_H_
#define PLAYERDBMANAGER_H_

#include "CharDefine.h"
#include "Common.h"
#include "Smart_Ptr.h"
#include "define.h"
#include "google/protobuf/message.h"
#include "MessageStruct/CharLogin/PlayerInfo.pb.h"

using namespace CommBaseOut;

class CStructTemplate
{
public:
	CStructTemplate(int type);
	~CStructTemplate();

	int SavePlayerBaseInfo(PlayerInfo::BaseInfo *info);

	int Save(int64 charid);

	void SetFlag(bool flag){ m_flag = flag; }

//	int SavePlayerBaseInfo(PlayerInfo::BaseInfo *info);

	void AddContent(Smart_Ptr<google::protobuf::Message> &content);
	void AddContent(google::protobuf::Message *content);

	void GetContent(google::protobuf::Message *info)
	{
		GUARD(CSimLock, obj, &m_structLock);

		info->CopyFrom(*m_content.Get());
	}

	//上线调用，不加锁
	Smart_Ptr<google::protobuf::Message> getContentNoLock(){return m_content;}

private:

	int m_type;
	CSimLock m_structLock;
	Smart_Ptr<google::protobuf::Message> m_content;
	bool m_flag;
};

class PlayerDBMgr
{
public:
	PlayerDBMgr();
	~PlayerDBMgr();

	int SavePlayerStruct(int64 charid);
	void SetFlag(int id, bool flag);

	void AddStruct(int type, Smart_Ptr<google::protobuf::Message> content);
	void AddStruct(int type, google::protobuf::Message *content);

	bool GetPlayerInfo(PlayerInfo::PlayerInfo *info);
	bool GetPlayerInfo(int id, google::protobuf::Message *content);

	//上线调用，不加锁
	Smart_Ptr<google::protobuf::Message> getProtobuf(ECharBaseStruct type);

private:
	Smart_Ptr<CStructTemplate> m_pStruct[eCharStructMax];
	DWORD64 m_beTime;
};

#endif /* PLAYERDBMANAGER_H_ */
