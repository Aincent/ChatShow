/*
 * EpollMain.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef NETWORK_EPOLLMAIN_H_
#define NETWORK_EPOLLMAIN_H_

#include <vector>
#include <map>
#include <deque>
#include "../Thread/Task.h"
#include "../define.h"
#include "Session.h"
#include "../Common.h"
#include "../Memory/MemAllocator.h"

using namespace std;

namespace CommBaseOut
{

class Context;
class Task;
class DispatchBase;

class CEpollLoop : public Task
#ifdef USE_MEMORY_POOL
, public MemoryBase
#endif
{
public:
	CEpollLoop(Context *c);
	~CEpollLoop();

	int Init(unsigned char index);

	int EpollCtl(int nOp, int nSocket, struct epoll_event *event);
	void DeleteSession(int fd, int err);
	void DeleteSessionEx(int fd, int err);
	DispatchBase *  GetDispatch()
	{
		return m_dispatch;
	}

	BYTE GetIndex() { 	return m_index; }
	int GetCount() { 	return m_session.GetCount(); }
	CSessionMgr *GetSessionMgr()
	{
		return &m_session;
	}

	void SendAccRequest(struct request_package &request, int len);
	void SendConnRequest(struct request_package &request, int len);

private:
	int SetMaxFile(int nMax);

	int SetNonBlocking(int nSock);

	int SetSockKeepAlive(int nSock, int nKeepIdle, int nKeepInterval, int nKeepCount);

	void ExeCommand();
	void ReadCommd(int fd);
	void BlockReadPipe(int pipefd, void *buffer, int sz);

private:
	virtual int svr();

private:
	Context *m_c;
	int m_epollFd;
	unsigned char m_index;
	CSessionMgr m_session;
	DispatchBase *  m_dispatch;
	int m_accfd[2];
	int m_confd[2];
	fd_set m_rfds;
};

class CEpollMain
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:
	CEpollMain(Context *c);

	 ~CEpollMain();

	int AddEpollLoop(unsigned char index);
	int DeleteAll();
	void AddChannel(int fd, int index);
	void DeleteChannel(int fd, int err);
	void DeleteChannelEx(int fd, int err);
	WORD GetEpollIndex(int fd);

	inline CEpollLoop * GetEpollLoop(int index)
	{
		if(index < 0)
			return 0;

		return m_epollLoop[index % m_epollLoop.size()];
	}

private:
	Context * m_c;
	vector<CEpollLoop *> m_epollLoop;
	map<int, WORD> m_channelEpoll;
	CRWLock m_channelLock;
};
}

#endif /* NETWORK_EPOLLMAIN_H_ */
