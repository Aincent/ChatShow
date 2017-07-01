/*
 * Context.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef NETWORK_CONTEXT_H_
#define NETWORK_CONTEXT_H_

#include <vector>
#include "NetWorkConfig.h"
#include "../Thread/Task.h"
#include "../Memory/MemAllocator.h"

using namespace std;

namespace CommBaseOut
{

class Message_Service_Handler;
class CAcceptorMgr;
class CConnMgr;
class CEpollMain;
class DispatchMgr;
class HandlerManager;
class GroupSession;

class Context
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:
	Context();

	 ~Context();

	int Init(Message_Service_Handler *mh, int blockThread, int ioThread);

	void AddAcceptor(AcceptorConfig &conf);
	void AddConnector(ConnectionConfig &conf);

	int Start();
	int Stop();

	void SystemErr(int fd, int lid, int ltype, int rid, int rtype, int err, string ip, int port);

	CAcceptorMgr * GetAccMgr()
	{
		return m_accMgr;
	}
	CConnMgr * GetConnMgr()
	{
		return m_connMgr;
	}
	HandlerManager *GetHandlerMgr()
	{
		return m_handlerMgr;
	}
	GroupSession *GetGroupSession()
	{
		return m_groupSession;
	}
	CEpollMain * GetEpollMgr()
	{
		return m_epoll;
	}
	Message_Service_Handler * GetServiceHandler()
	{
		return m_connHandler;
	}
	AcceptorConfig * GetAccConfig(int index)
	{
		return &m_acceptAdded[index];
	}
	vector<ConnectionConfig> &GetAllConnConfig()
	{
		return m_connAdded;
	}
	ConnectionConfig * GetConnConfig(int index)
	{
		return &m_connAdded[index];
	}
	DispatchMgr * GetDispatch()
	{
		return m_dispatchMgr;
	}
	ChannelConfig * GetConfig()
	{
		if(m_connAdded.size() == 0)
		{
			return static_cast<ChannelConfig *>(&m_acceptAdded[0]);
		}

		return static_cast<ChannelConfig *>(&m_connAdded[0]);
	}

	ChannelConfig * GetConfig(unsigned char index, unsigned char type)
	{
		if(type == eConnEpoll)
		{
			return static_cast<ChannelConfig *>(&m_connAdded[index]);
		}

		return static_cast<ChannelConfig *>(&m_acceptAdded[index]);
	}

private:

	vector<ConnectionConfig> m_connAdded;
	vector<AcceptorConfig> m_acceptAdded;

	CAcceptorMgr *m_accMgr;
	CConnMgr *m_connMgr;
	CEpollMain *m_epoll;
	Message_Service_Handler *m_connHandler;
	DispatchMgr * m_dispatchMgr;
	HandlerManager *m_handlerMgr;
	GroupSession * m_groupSession;

	int m_blockThread;
	int m_ioThread;
};
}

#endif /* NETWORK_CONTEXT_H_ */
