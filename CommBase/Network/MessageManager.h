/*
 * MessageManager.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef NETWORK_MESSAGEMANAGER_H_
#define NETWORK_MESSAGEMANAGER_H_

#include <deque>
#include "../Smart_Ptr.h"
#include "../Thread/Task.h"
#include "../define.h"
#include "NetWorkConfig.h"
#include "../Common.h"
#include "Service_Handler.h"
#include "../ClassBase.h"
#include "../Ref_Object.h"
#include "../Memory/MemAllocator.h"

using namespace std;

namespace CommBaseOut
{

class Context;
class Session;

class Message : public Ref_Object
#ifdef USE_MEMORY_POOL
, public MemoryBase
#endif
{
public:
	Message();
	Message(int id, unsigned int type);
	Message(int id, unsigned int type, int channel);
	Message(int channel);
	Message(int id, unsigned int type, int group, int64 key);
	Message(Safe_Smart_Ptr<Message> &message);
	~Message();

	void SetHead(packetHeader &head);
	void GetHead(packetHeader *head);
	void SetContent(const char *content, int len);
	char * GetContent();

	void SetAddr(Safe_Smart_Ptr<Inet_Addr> &addr)
	{
		m_addr = addr;
	}

	Safe_Smart_Ptr<Inet_Addr> &GetAddr()
	{
		return m_addr;
	}

	void SetErrno(int no)
	{
		m_errno = no;
	}
	int GetErrno()
	{
		return m_errno;
	}

	void SetReqID(int id)
	{
		m_reqID = id;
	}

	int GetReqID()
	{
		return m_reqID;
	}

	void SetChannelID(int id)
	{
		m_channelID = id;
	}

	int GetChannelID()
	{
		return m_channelID;
	}

	void SetMessageID(DWORD id)
	{
		m_messageID = id;
	}

	DWORD GetMessageID()
	{
		return m_messageID;
	}

	int GetLength()
	{
		return m_length;
	}

	void SetMessageType(unsigned char type)
	{
		m_messageType = type;
	}

	int GetMessageType()
	{
		return m_messageType;
	}

	char * GetBuffer(int &len)
	{
		len = m_length;
		return m_content;
	}

	void SetRequest(Safe_Smart_Ptr<Message> &req)
	{
		m_req = req;
	}
	Safe_Smart_Ptr<Message> &GetRequest()
	{
		return m_req;
	}

	void SetTimeout(WORD time)
	{
		m_timeout = time;
	}

	WORD GetTimeout()
	{
		return m_timeout;
	}

	void SetSecurity(bool se)
	{
		m_security = se;
	}
	bool GetSecurity()
	{
		return m_security;
	}

	void SetRemoteID(short int id)
	{
		m_remoteID = id;
	}

	short int GetRemoteID()
	{
		return m_remoteID;
	}

	void SetRemoteType(unsigned char type)
	{
		m_remoteType = type;
	}

	unsigned char GetRemoteType()
	{
		return m_remoteType;
	}

	short int GetLocalID()
	{
		return m_localID;
	}
	void SetLocalID(short int id)
	{
		m_localID = id;
	}

	void SetLocalType(unsigned char type)
	{
		m_localType = type;
	}
	unsigned char GetLocalType()
	{
		return m_localType;
	}

	void SetMessageTime(DWORD64 time)
	{
		m_sendTime = time;
	}

	DWORD64 GetMessageTime()
	{
		return m_sendTime;
	}

	int GetLoopIndex()
	{
		return m_loopIndex;
	}
	void SetLoopIndex(int index)
	{
		m_loopIndex = index;
	}

	int GetGroup()
	{
		return m_group;
	}
	void SetGroup(int group)
	{
		m_group = group;
	}

	int64 GetGroupKey()
	{
		return m_groupKey;
	}
	void SetGroupKey(int64 key)
	{
		m_groupKey = key;
	}

	void SetAct(Safe_Smart_Ptr<NullBase> act)
	{
		m_act = act;
	}

	Safe_Smart_Ptr<NullBase> & GetAct()
	{
		return m_act;
	}

	//加密判断
	void EncryptMessage();
	//解密判断
	bool UnEncryptMessage();


private:
	char * m_content;

	Safe_Smart_Ptr<Message> m_req;
	Safe_Smart_Ptr<Inet_Addr> m_addr;

	short int m_remoteID;
	unsigned char m_remoteType;

	short int m_localID;
	unsigned char m_localType;

	WORD m_timeout;
	int m_reqID;
	int m_channelID;
	DWORD m_messageID;
	BYTE m_messageType;
	int m_length;

	int m_errno;
	bool m_security;
	DWORD64 m_sendTime;
	int m_loopIndex;
	int m_group;
	int64 m_groupKey;

	Safe_Smart_Ptr<NullBase> m_act;
};
}

#endif /* NETWORK_MESSAGEMANAGER_H_ */
