/*
 * AcceptManager.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "AcceptManager.h"
#include "Context.h"
#include <sys/socket.h>
#include "InetAddress.h"
#include "../define.h"
#include "EpollMain.h"
#include "Session.h"
#include "Service_Handler.h"
#include "../Log/Logger.h"

namespace CommBaseOut
{

	CAcceptor::CAcceptor(Context *c):m_sockLis(-1),m_c(c),m_index(0)
	{
	}
	CAcceptor::~CAcceptor()
	{
		if(m_sockLis != -1)
			close(m_sockLis);
	}

	int CAcceptor::Init(AcceptorConfig& conf)
	{
		m_sockLis = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if(m_sockLis == -1)
		{
			LOG_BASE(FILEINFO, "create socket failed");

			return eCreateSockErr;
		}

		int nOpt = 1;
		struct timeval timeo = {1, 0};
		socklen_t tilen = sizeof(timeo);

		setsockopt(m_sockLis, SOL_SOCKET, SO_REUSEADDR, (const void*)&nOpt, sizeof(nOpt));
		setsockopt(m_sockLis, SOL_SOCKET, SO_RCVTIMEO, &timeo, tilen);
		setsockopt(m_sockLis, IPPROTO_TCP, TCP_NODELAY, (const void*)&nOpt, sizeof(nOpt));

		struct sockaddr_in addr;

		addr.sin_family = AF_INET;
		addr.sin_port = htons(conf.listen_port_);
		if(conf.listen_ip_ == "")
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
		else
		addr.sin_addr.s_addr = inet_addr(conf.listen_ip_.c_str());

		if(bind(m_sockLis, (struct sockaddr *)&addr, sizeof(sockaddr_in)) < 0)
		{
			LOG_BASE(FILEINFO, "bind socket failed");

			close(m_sockLis);
			return eBindErr;
		}

		if(listen(m_sockLis, MAX_LISTEN_SIZE) < 0)
		{
			LOG_BASE(FILEINFO, "listen socket failed");

			close(m_sockLis);
			return eListenErr;
		}

		m_addr = conf;

		return eNetSuccess;
	}

	int CAcceptor::svr()
	{
		socklen_t socklen;
		struct sockaddr_in addrClient;
		socklen = sizeof(struct sockaddr);

		while(!m_flag)
		{
			int nSocketClient = accept(m_sockLis, (struct sockaddr *)&addrClient, &socklen);
			if(nSocketClient == -1)
			{
				CUtil::MSleep(100);
				continue;
			}

			linger sockLinger;

			sockLinger.l_onoff = 1;
			sockLinger.l_linger = 0;
			setsockopt(nSocketClient, SOL_SOCKET, SO_LINGER, (const char*)&sockLinger, sizeof(linger));

			int nOpt = 1;

			setsockopt(nSocketClient, SOL_SOCKET, SO_REUSEADDR, (const void*)&nOpt, sizeof(nOpt));
			setsockopt(nSocketClient, IPPROTO_TCP, TCP_NODELAY, (const void*)&nOpt, sizeof(nOpt));

			CEpollLoop * loop = m_c->GetEpollMgr()->GetEpollLoop(nSocketClient);
			if(loop == 0)
			{
				LOG_BASE(FILEINFO, " accept Client socket but get epollLoop error ip[%s] ", inet_ntoa(addrClient.sin_addr));
				CloseSocket(nSocketClient);

				continue;
			}
			else
			{
				request_package rPackage;

				rPackage.u.listen.fd = nSocketClient;
				CUtil::SafeMemmove(rPackage.u.listen.host, 16, inet_ntoa(addrClient.sin_addr), strlen(inet_ntoa(addrClient.sin_addr)));
				rPackage.u.listen.index = m_index;
				rPackage.u.listen.opaque = ntohs(addrClient.sin_port);

				rPackage.header[6] = eRequestListen;
				rPackage.header[7] = sizeof(rPackage.u.listen);

				loop->SendAccRequest(rPackage, sizeof(rPackage.u.listen) + 2);
			}
		}

		return eNetSuccess;
	}

	CAcceptorMgr::CAcceptorMgr(Context *c):m_c(c)
	{

	}

	CAcceptorMgr::~CAcceptorMgr()
	{
		vector<CAcceptor *>::iterator it = m_acceptor.begin();
		for(; it!=m_acceptor.end(); ++it)
		{
			(*it)->End();
			delete (*it);
			(*it) = 0;
		}
	}

	int CAcceptorMgr::AddAccept(AcceptorConfig &conf)
	{
		int res = -1;
		CAcceptor *tmp = NEW CAcceptor(m_c);

		if((res=tmp->Init(conf)) != eNetSuccess)
		{
			return res;
		}

		if((res=tmp->Start(1)) != 0)
		{
			LOG_BASE(FILEINFO, "accept start failed");

			return res;
		}

		tmp->SetIndex(m_acceptor.size());
		m_acceptor.push_back(tmp);

		return eNetSuccess;
	}

	int CAcceptorMgr::DeleteAll()
	{
		vector<CAcceptor *>::iterator it = m_acceptor.begin();
		for(; it!=m_acceptor.end(); ++it)
		{
			(*it)->End();
			delete (*it);
			(*it) = 0;
		}

		m_acceptor.clear();

		return eNetSuccess;
	}
}

