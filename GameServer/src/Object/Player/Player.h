/*
 * Player.h
 *
 *  Created on: Jun 27, 2017
 *      Author: root
 */

#ifndef PLAYER_H_
#define PLAYER_H_

#include <map>
#include "../Creature.h"
#include "Smart_Ptr.h"
#include "util.h"
#include "SvrConfig.h"
#include "./google/protobuf/message.h"
#include "CharDefine.h"
#include "Timer/TimerInterface.h"
#include "Network/MessageManager.h"
#include "MessageStruct/CharLogin/PlayerInfo.pb.h"
#include "MessageStruct/CharLogin/CharBaseInfo.pb.h"
#include "../Creature.h"

using namespace std;
using namespace CommBaseOut;

class FriendMgr;

struct SaveDataInfo
{
	SaveDataInfo()
	{
		msgtype = 0;
		msgid = 0;
		count = 0;
		message = NULL;
		content = NULL;
		m_act   = NULL;
	}
	int msgtype;//消息类型
	int msgid;//消息ID
	Safe_Smart_Ptr<CommBaseOut::Message> message;//与通讯对方的信息
	Smart_Ptr<google::protobuf::Message> content;//消息内容
	Safe_Smart_Ptr<NullBase> m_act;
	int count;//次数
};

class Player : public CreatureObj
{
public:
	Player(eObjType type);
	~Player();

	void ReleasePlayer();

	void SetMyself(const Smart_Ptr<Player> &player)
	{
		if(!player)
		{
			cout<<endl<<"Player Set Myself is NULL,Player is %lld"<<GetID();
		}

		m_myself = player;
	}

	Smart_Ptr<Player> &GetMyself() { 	return m_myself; }
	Smart_Ptr<CreatureObj> GetCreatureSelf() { 	return m_myself; }

	void InitBaseInfo(const PlayerInfo::BaseInfo &info);
	void SetBaseInfo(PlayerInfo::BaseInfo *info);

	void SetCharBaseInfo(CharLogin::CharBaseInfo *info);

	bool GetSex() { 	return m_sex; }
	void SetSex(bool sex) { 	m_sex = sex; }

	void SetChannelID(int channelID) {	 m_channelID=channelID; }
	int GetChannelID() {	 return m_channelID; }

	void SetDBChannelID(int channelID) {	 m_dbChannelID=channelID; }
	int GetDBChannelID() {	 return m_dbChannelID; }

	void SetOffLineTime(const int64 &time)
	{
		m_offineTime = time;
	}

	int64 GetOffLineTime()
	{
		return m_offineTime;
	}

	void SetOnlineTime(const int64 &time)
	{
		m_onlineTime = time;
	}

	void SetLoginTime(const int64 &time)
	{
		m_loginTime = time;
	}

	int64 GetLoginTime()
	{
		return m_loginTime;
	}

	string &GetIP(){return m_ip;	}
	void SetIP(const string & ip) {  m_ip = ip; }

	string& GetWchatname()
	{
		return m_wchatname;
	}

	void SetWchatname(const string& wchatname)
	{
		m_wchatname = wchatname;
	}

	string& GetHead()
	{
		return m_head;
	}

	void SetHead(const string& head)
	{
		m_head = head;
	}

	int GetCountry()
	{
		return m_country;
	}

	void SetCountry(const int& country)
	{
		m_country = country;
	}

	int GetProvince()
	{
		return m_province;
	}

	void SetProvince(const int& province)
	{
		m_province = province;
	}

	int GetCity()
	{
		return m_city;
	}

	void SetCity(const int& city)
	{
		m_city = city;
	}

	string& GetSignature()
	{
		return m_signature;
	}

	void SetSignature(const string& signature)
	{
		m_signature = signature;
	}

	uint GetMoney()
	{
		return m_money;
	}

	void SetMoney(const int& money)
	{
		m_money = money;
	}

	uint GetTurnMoney()
	{
		return m_turnmoney;
	}

	void SetTurnMoney(const int& turnmoney)
	{
		m_turnmoney = turnmoney;
	}

	string& GetQRCode()
	{
		return m_qrcode;
	}

	void SetQRCode(const string& qrcode)
	{
		m_qrcode = qrcode;
	}

	string& GetPhone()
	{
		return m_phone;
	}

	void SetPhone(const string& phone)
	{
		m_phone = phone;
	}

	int GetYear()
	{
		return m_year;
	}

	void SetYear(const int& year)
	{
		m_year = year;
	}

	int GetMonth()
	{
		return m_month;
	}

	void SetMonth(const int& month)
	{
		m_month = month;
	}

	int GetDay()
	{
		return m_day;
	}

	void SetDay(const int& day)
	{
		m_day = day;
	}

	void SetDataFlag(int type) { 	m_dirtyData[type] = true; }

	void SaveAllPlayer(void * obj);
	void SaveAllPlayer(SaveDataInfo& sendData, bool flag = false, bool isDel = false);
	void SendToSaveData(PlayerInfo::SaveTypeInfo *info);
	void SendToSaveData(PlayerInfo::SaveTypeInfo *info, SaveDataInfo& sendData);

	void SetInitPartDataFlag(int type);

	void SetTypeValue(CharLogin::SynCharBase& attr);

	Smart_Ptr<FriendMgr>& GetFriendManager()
	{
		return m_friendMgr;
	}
private:
	bool m_sex; // 性别

	int m_channelID;
	int m_dbChannelID;

	int64 m_offineTime; //玩家下线时间
	int64 m_onlineTime; //玩家在线时长
	int64 m_totalOnlineTime;  //总在线时长

	int64 m_loginTime; //玩家登录时间
	string m_ip;  //登录IP

	string m_wchatname;
	string m_head;
	int m_country;
	int m_province;
	int m_city;
	string m_signature;
	uint m_money;
	uint m_turnmoney;
	string m_qrcode;
	string m_phone;
	int m_year;
	int m_month;
	int m_day;

	Smart_Ptr<Player> m_myself;

	Smart_Ptr<FriendMgr> m_friendMgr;

	bool m_dirtyData[eCharStructMax];
	bool m_initpartData[eCharStructMax];  //玩家组件信息初始化标识位 防止频繁上下线

	TimerConnection m_timerID;
};


#endif /* PLAYER_H_ */
