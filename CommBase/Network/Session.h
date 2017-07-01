/*
 * Session.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef NETWORK_SESSION_H_
#define NETWORK_SESSION_H_

#include "../define.h"
#include <map>
#include "../Common.h"
#include "../Thread/Task.h"
#include "../Smart_Ptr.h"
#include "NetWorkConfig.h"
#include "InetAddress.h"
#include "CircleBuffer.h"
#include "../Ref_Object.h"
#include "../Singleton.h"
#include "../Memory/MemAllocator.h"
#include <unistd.h>

using namespace std;

namespace CommBaseOut
{

class Inet_Addr;
class Context;
class Message;
class CCircleBuff;
class CSessionMgr;

class Session : public Ref_Object
#ifdef USE_MEMORY_POOL
, public MemoryBase
#endif
{
	public:
	Session(Context *c, CSessionMgr *s, int sock, int flag, struct sockaddr_in &addr, bool isSecurity);
	~Session();

	int Recv();
	int Recv(deque<Safe_Smart_Ptr<Message> > &vec);
	int Write(char * content, packetHeader *head);
	int Write(char *buf, int len);
	int Write(Safe_Smart_Ptr<Message> &message);
	int Send();

	void ResetSendBuf()
	{
		bufSend->ResetSize(MAX_NET_MSG_SIZE[eServerBuf]);
	}

	int AddMessage(Safe_Smart_Ptr<Message> &message);

	char * GetRecvBuf();
	DWORD GetRecvPos();

	void AddPos(DWORD len);
	void SetPos(DWORD len);

	void SetRemoteID(short int id);
	void SetRemoteType(unsigned char type);

	short int GetRemoteID()
	{
		return m_remoteID;
	}
	unsigned char GetRemoteType()
	{
		return m_remoteType;
	}

	int GetSock()
	{
		return m_accSock;
	}

	void closeSocket()
	{
		close(m_accSock);
//		m_accSock = -1;
	}

	int GetFlag();

	bool GetSecurity()
	{
		return m_security;
	}

	Safe_Smart_Ptr<Inet_Addr> &GetAddr()
	{
		return m_addr;
	}

	void SetGroupID(int id)
	{
		m_groupID = id;
	}
	int GetGroupID()
	{
		return m_groupID;
	}

private:

	Context *m_c;
	short int m_remoteID;
	unsigned char m_remoteType;
	Safe_Smart_Ptr<Inet_Addr> m_addr;
	int m_accSock;
	DWORD64 m_lastPacketTime;
	int m_flag;
	char *m_bufRecv;
	DWORD dwRecvPos;
	CSessionMgr * m_s;
	bool m_security;

	CCircleBuff *bufSend;
	CSimLock m_bufLock;

	int m_groupID;

#ifdef USE_PACKAGE_COUNT
	int m_recvCount;
	int m_sendCount;
	int64 m_recvTime;
	int64 m_sendTime;
#endif
};

typedef struct sEpollEventPtr
{
	Safe_Smart_Ptr<Session> se;
}EpollEventPtr;

class CSessionMgr
{
public:
	CSessionMgr(Context *c);
	~CSessionMgr();

	EpollEventPtr * AddTimerSessionEx(int sock, struct sockaddr_in &addr, ChannelConfig * config, WORD index, unsigned char type);
	int ConnectSuccess(Safe_Smart_Ptr<Message> &message);

	int Write(Safe_Smart_Ptr<Message> &message);
	void AddMessage(int channel, deque<Safe_Smart_Ptr<Message> > &messageDeque);
	void AddMessage(Safe_Smart_Ptr<Message> message);

	short int GetRemoteID(int fd);
	unsigned char GetRemoteType(int fd);
	Safe_Smart_Ptr<Inet_Addr> GetAddr(int fd);

	Safe_Smart_Ptr<Session> GetSession(int fd)
	{
		if(fd < 0)
		{
			return 0;
		}

		GUARD_READ(CRWLock, obj, &m_seLock);
		map<int, Safe_Smart_Ptr<Session> >::iterator it = m_session.find(fd);
		if(it != m_session.end())
		{
			return it->second;
		}
		obj.UnLock();

		GUARD_READ(CRWLock, objTime, &m_timeSeLock);
		map<int, Safe_Smart_Ptr<Session> >::iterator itTime = m_timeOutSe.find(fd);
		if(itTime != m_timeOutSe.end())
		{
			return itTime->second;
		}

		return 0;
	}

	void DeleteAll();
	void DeleteSession(int fd, int err);
	void DeleteSessionEx(int fd, int err);
	int GetCount()
	{
		int size = 0;

		GUARD_READ(CRWLock, obj, &m_seLock);
		size += m_session.size();
		obj.UnLock();

		GUARD_READ(CRWLock, objTime, &m_timeSeLock);
		size += m_timeOutSe.size();

		return size;
	}

	EpollEventPtr * GetEventPtr(int fd)
	{
		if(fd >= MAX_EPOLL_EVENT)
			return 0;

		return &m_epollPtr[fd];
	}

	void Tick();

private:
	map<int, Safe_Smart_Ptr<Session> > m_session;
	CRWLock m_seLock;
	map<int, Safe_Smart_Ptr<Session> > m_timeOutSe;
	CRWLock m_timeSeLock;

	map<int, int64> m_timeout;
	int64 m_bTime;
	int m_tick;

	EpollEventPtr *m_epollPtr;
	Context *m_c;
};

}

#endif /* NETWORK_SESSION_H_ */
