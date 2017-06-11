/*
 * Context.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "Context.h"
#include "AcceptManager.h"
#include "Service_Handler.h"
#include "EpollMain.h"
#include "DispatchMessage.h"
#include "ConnManager.h"
#include "Session.h"
#include "../util.h"
#include "GroupSession.h"
#include "MessageManager.h"
#include "NetWorkConfig.h"

namespace CommBaseOut
{

Context::Context():m_accMgr(0),m_connMgr(0), m_epoll(0),//m_messageRecv(0),
		m_connHandler(0),m_handlerMgr(0),m_blockThread(1),m_ioThread(1)
{
	m_accMgr = NEW CAcceptorMgr(this);
	m_connMgr = NEW CConnMgr(this);
	m_epoll = NEW CEpollMain(this);
	m_dispatchMgr = NEW DispatchMgr(this);
	m_handlerMgr = NEW	HandlerManager(this);
	m_groupSession = NEW GroupSession(this);
}
Context::~Context()
{
	m_connAdded.clear();
	m_acceptAdded.clear();

	if(m_accMgr)
	{
		delete m_accMgr;
		m_accMgr = 0;
	}

	if(m_connMgr)
	{
		m_connMgr->End();
		delete m_connMgr;
		m_connMgr = 0;
	}

	if(m_epoll)
	{
		delete m_epoll;
		m_epoll = 0;
	}

	if(m_dispatchMgr)
	{
		delete m_dispatchMgr;
		m_dispatchMgr = 0;
	}

	if(m_handlerMgr)
	{
		delete m_handlerMgr;
		m_handlerMgr = 0;
	}

	if(m_groupSession)
	{
		delete m_groupSession;
		m_groupSession = 0;
	}
}

int Context::Init(Message_Service_Handler *mh, int blockThread, int ioThread)
{
	m_blockThread = blockThread;
	if(m_blockThread <= 0)
		m_blockThread = 1;

	m_ioThread = ioThread;
	m_connHandler = mh;

	return eNetSuccess;
}

void Context::AddAcceptor(AcceptorConfig &conf)
{
	m_acceptAdded.push_back(conf);
}

void Context::AddConnector(ConnectionConfig &conf)
{
	m_connAdded.push_back(conf);
}

int Context::Start()
{
	int i=0;
	int res = -1;

	m_dispatchMgr->Init(m_blockThread);

	for(i=0; i<m_ioThread; ++i)
	{
		if((res=m_epoll->AddEpollLoop(i)) != eNetSuccess)
		{
			return res;
		}
	}

	vector<AcceptorConfig>::iterator itAcc = m_acceptAdded.begin();
	for(; itAcc!=m_acceptAdded.end(); ++itAcc)
	{
		if((res=m_accMgr->AddAccept(*itAcc)) != eNetSuccess)
		{
			return res;
		}
	}

	if((res=m_connMgr->Init()) != eNetSuccess)
	{
		return res;
	}

	if(m_connAdded.size() > 0)
	{
		if((res=m_connMgr->Start(1)) != eNetSuccess)
		{
			return res;
		}
	}

	return eNetSuccess;
}

int Context::Stop()
{
	m_connMgr->Close();
	m_accMgr->DeleteAll();
	m_epoll->DeleteAll();
	m_dispatchMgr->DeleteAll();

	return eNetSuccess;
}

void Context::SystemErr(int fd, int lid, int ltype, int rid, int rtype, int err, string ip, int port)
{
	int id = -1;

	switch(err)
	{
		case eConnErr:
		case eAddSeTimeoutErr:
		case eGetSocketErr:
		case eSetSocketErr:
		case eCtlEpollErr:
		case eSessionAdded:
		case eNotTimeOut:
		{
			id = eConnFailMessage;
			break;
		}
		case eEpollClose:
		case eEpollNull:
		case eConMessageErr:
		case eConnPacketErr:
		case eSessionTimeout:
		case eSocketClose:
		{
			id = eConnErrMessage;
			break;
		}
		case eNetSuccess:
		{
			id = eConnBuildSuccess;
			break;
		}
	}

	Safe_Smart_Ptr<Message> message = NEW Message();
	char con[128] = {0};
	SystemMessageCon content;

	bzero(&content, sizeof(content));
	content.channel = fd;
	content.lid = lid;
	content.ltype = ltype;
	content.rid = rid;
	content.rtype = rtype;
	content.err = err;
	CUtil::SafeMemmove(content.ip, 16, ip.c_str(), ip.size());
	content.port = port;

	CUtil::SafeMemmove(con, 128, &content, sizeof(content));
	message->SetMessageID(id);
	message->SetContent(con, sizeof(content));
	message->SetMessageType(SystemMessage);

//	CEpollLoop * loop = m_epoll->GetEpollLoop(0);
//	if(loop == 0)
//		return;
//
//	loop->GetDispatch()->AddMessage(message);
	m_dispatchMgr->GetDispatch(fd)->AddMessage(message);
}
}
