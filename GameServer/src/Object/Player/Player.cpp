/*
 * Player.cpp
 *
 *  Created on: Jun 27, 2017
 *      Author: root
 */
#include "Player.h"
#include "Network/Message_Facade.h"
#include "ServerMsgDefine.h"
#include "../../ServerManager.h"
#include "Log/Logger.h"
#include "../../MessageBuild.h"
#include "MessageStruct/ServerReturnIntChar.pb.h"
#include "MessageStruct/ServerReturn2Int.pb.h"
#include "MessageStruct/ServerReturnInt.pb.h"
#include "PlayerMessage.h"
#include "MsgCommonClass.h"
#include "../../Friend/FriendMgr.h"

Player::Player(eObjType type) : CreatureObj(type),m_sex(false),m_channelID(-1),m_dbChannelID(-1),m_offineTime(0)
,m_onlineTime(0),m_totalOnlineTime(0),m_loginTime(0),m_ip(""),m_wchatname(""),m_head(""),m_country(0),m_province(0),m_city(0),m_signature(""),
m_money(0),m_turnmoney(0),m_qrcode(""),m_phone(""),m_year(0),m_month(0),m_day(0),m_timerID(0)
{
	m_friendMgr = new FriendMgr(this);
}

Player::~Player()
{
}

void Player::ReleasePlayer()
{
	if (m_timerID.IsValid())
	{
		m_timerID.SetNull();
	}
}

void Player::InitBaseInfo(const PlayerInfo::BaseInfo &info)
{
	SetID(info.charid());
	SetName(info.charname());
	SetWchatname(info.wchatname());
	SetHead(info.head());
	SetSex(info.sex());
	SetCountry(info.country());
	SetProvince(info.province());
	SetCity(info.city());
	SetSignature(info.signature());
	SetMoney(info.money());
	SetTurnMoney(info.turnmoney());
	SetQRCode(info.qrcode());
	SetPhone(info.phone());
	SetYear(info.year());
	SetMoney(info.month());
	SetDay(info.day());

	this->SetInitPartDataFlag(eBaseInfo);
}

void Player::SetBaseInfo(PlayerInfo::BaseInfo *info)
{
	info->set_charid(GetID());
	info->set_charname(GetName());
	info->set_wchatname(GetWchatname());
	info->set_head(GetHead());
	info->set_sex(GetSex());
	info->set_country(GetCountry());
	info->set_province(GetProvince());
	info->set_city(GetCity());
	info->set_signature(GetSignature());
	info->set_money(GetMoney());
	info->set_turnmoney(GetTurnMoney());
	info->set_qrcode(GetQRCode());
	info->set_phone(GetPhone());
	info->set_year(GetYear());
	info->set_month(GetMonth());
	info->set_day(GetDay());
}

void Player::SetCharBaseInfo(CharLogin::CharBaseInfo *info)
{
	info->set_charid(GetID());
	info->set_charname(GetName());
	info->set_wchatname(GetWchatname());
	info->set_head(GetHead());
	info->set_sex(GetSex());
	info->set_country(GetCountry());
	info->set_province(GetProvince());
	info->set_city(GetCity());
	info->set_signature(GetSignature());
	info->set_money(GetMoney());
	info->set_turnmoney(GetTurnMoney());
	info->set_qrcode(GetQRCode());
	info->set_phone(GetPhone());
	info->set_year(GetYear());
	info->set_month(GetMonth());
	info->set_day(GetDay());
}

void Player::SetInitPartDataFlag(int type)
{
	if (type < eBaseInfo || type >= eCharStructMax)
		return;
	if (m_initpartData[type])
		return;

	//玩家 所有 有关数据  全部  初始化 完成才 启动  定时器(目前 是 eFashionInfo 最后 完成  初始化)
	if (type == eCharStructMax - 1)
	{
		if (m_timerID.IsValid())
		{
			m_timerID.SetNull();
		}
		m_timerID = TimerInterface::GetInstance()->RegRepeatedTime(&Player::SaveAllPlayer, this, 0, 0, 0, 0, SAVE_CHARCACHE_TODB,SAVE_CHARCACHE_TODB);
	}
	m_initpartData[type] = true;
}

void Player::SaveAllPlayer(void * obj)
{
	SaveDataInfo sendData;
	SaveAllPlayer(sendData, false, false);
}


void Player::SaveAllPlayer(SaveDataInfo& sendData, bool flag, bool isDel)
{
	vector<PlayerInfo::SaveTypeInfo> infoVec;

	PlayerInfo::SaveTypeInfo info;
	PlayerInfo::SaveTypeInfo tmpInfo;

	DWORD64 type = 0;
	DWORD64 leftValue = 1;

	for (unsigned int i = 0; i < eCharStructMax; ++i)
	{
		if ((m_dirtyData[i] || flag) && (m_initpartData[i]))
		{
			type |= (leftValue << i);

			switch (i)
			{
				case eBaseInfo:
				{

					break;
				}
				default:
				{
					LOG_ERROR(FILEINFO, "timer save player info but type is error");
				}
			}

			m_dirtyData[i] = false;

			if (info.ByteSize() > NORMAL_MSG_SEND_SIZE)
			{
				info.set_type(type);
				info.set_id(GET_PLAYER_CHARID(GetID()));
				info.set_isdel(false);
				type = 0;
				infoVec.push_back(info);
				info.CopyFrom(tmpInfo);  //不能直接用Clear()
			}
		}
	}

	if (type > 0)
	{
		info.set_type(type);
		info.set_id(GET_PLAYER_CHARID(GetID()));
		info.set_isdel(false);
		infoVec.push_back(info);
		info.CopyFrom(tmpInfo);
	}

	sendData.count = infoVec.size();

	int sizeFlag = (int)infoVec.size() - 1;
	for(int i = 0; i < sendData.count; ++i)
	{
		PlayerInfo::SaveTypeInfo sendtmpInfo;
		sendtmpInfo.CopyFrom(infoVec[i]);

		if (i == sizeFlag)
			sendtmpInfo.set_isdel(isDel);

		if(sendData.msgtype==0)
		{
			SendToSaveData(&sendtmpInfo);
		}
		else
		{
			SendToSaveData(&sendtmpInfo,sendData);
		}
	}

	//下线时没有任何改变也要发送
	if(isDel && sendData.count==0)
	{
		PlayerInfo::SaveTypeInfo sendtmpInfo;
		sendtmpInfo.set_type(0);
		sendtmpInfo.set_id(GET_PLAYER_CHARID(GetID()));
		sendtmpInfo.set_isdel(isDel);
		SendToSaveData(&sendtmpInfo, sendData);
	}

	if(flag)
	{
		//玩家下线 保存数据 同时  释放 资源
		this->ReleasePlayer();
	}

	//如果没有数据没改变，请求的消息要返回
	if(sendData.count==0 && sendData.msgtype > 0)
	{
		PlayerMessage::GetInstance()->dealWithSaveResult(sendData);
	}

}

void Player::SendToSaveData(PlayerInfo::SaveTypeInfo *info)
{
	Safe_Smart_Ptr<CommBaseOut::Message> messRet = build_message(MSG_SIM_GT2GM_SAVE_PLAYERINFO, info, m_dbChannelID, SimpleMessage);
	Message_Facade::Send(messRet);
}

void Player::SendToSaveData(PlayerInfo::SaveTypeInfo *info, SaveDataInfo& sendData)
{
	PlayerMessage::GetInstance()->addSendDataCach(GetID(), sendData);
	Safe_Smart_Ptr<CommBaseOut::Message> messRet = build_message(MSG_REQ_GS2DB_SAVE_PLAYERDATA, info, m_dbChannelID, Request);
	messRet->SetAct(new IntAct(GetID()));
	Message_Facade::Send(messRet);
}

void Player::SetTypeValue(CharLogin::SynCharBase& attr)
{
	for(int i = 0; i < attr.attr_size(); i++)
	{
		CharLogin::SynCharAttr synAttr = attr.attr(i);
		switch(synAttr.type())
		{
		case eCharName:
			{
				SetName(synAttr.attrstr());
				break;
			}
		case eWchatName:
			{
				SetWchatname(synAttr.attrstr());
				break;
			}
		case eHeadPhoto:
			{
				SetHead(synAttr.attrstr());
				break;
			}
		case eCharSex:
			{
				SetSex(synAttr.attrint());
				break;
			}
		case eCountry:
			{
				SetCountry(synAttr.attrint());
				break;
			}
		case eProvince:
			{
				SetProvince(synAttr.attrint());
				break;
			}
		case eCity:
			{
				SetCity(synAttr.attrint());
				break;
			}
		case eCharSignature:
			{
				SetSignature(synAttr.attrstr());
				break;
			}
		case eCharTurnMoney:
			{
				SetTurnMoney(synAttr.attrint());
				break;
			}
		case eQRCode:
			{
				SetQRCode(synAttr.attrstr());
				break;
			}
		case ePhone:
			{
				SetPhone(synAttr.attrstr());
				break;
			}
		case eYear:
			{
				SetYear(synAttr.attrint());
				break;
			}
		case eMonth:
			{
				SetMonth(synAttr.attrint());
				break;
			}
		case eDay:
			{
				SetDay(synAttr.attrint());
				break;
			}
		}
	}
}

