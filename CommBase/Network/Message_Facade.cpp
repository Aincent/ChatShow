/*
 * Message_Facade.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "Message_Facade.h"
#include "Context.h"
#include "DispatchMessage.h"
#include "MessageManager.h"
#include "Session.h"
#include "EpollMain.h"
#include "../Memory/MemAllocator.h"
#include "GroupSession.h"
#include "../Log/Logger.h"

namespace CommBaseOut
{

Context *Message_Facade::m_c = 0;
Event Message_Facade::m_e;

int Message_Facade::Init(Message_Service_Handler *mh, int blockThread, int ioThread)
{
	if(m_c == 0)
	{
		m_c = NEW Context();
	}

	m_c->Init(mh, blockThread, ioThread);

	return eNetSuccess;
}

void Message_Facade::AddAcceptConfig(AcceptorConfig &conf)
{
	m_c->AddAcceptor(conf);
}
void Message_Facade::AddConnectionConfig(ConnectionConfig &conf)
{
	m_c->AddConnector(conf);
}

int Message_Facade::Run()
{
	return m_c->Start();
}

void Message_Facade::ProcessMessage()
{
	DispatchBase * tmp = m_c->GetDispatch()->GetDispatch(0);

	if(tmp == 0)
	{
		return;
	}

	tmp->ProcessMessage();
}

void Message_Facade::MessageFlush()
{
	DispatchBase * tmp = m_c->GetDispatch()->GetDispatch(0);

	if(tmp == 0)
	{
		return;
	}

	tmp->MessageFlush();
}

void Message_Facade::Send(Safe_Smart_Ptr<Message> &message)
{
	int channel = message->GetChannelID();
	if(!message || channel < 0 || channel >= MAX_EPOLL_fd)
		return;

#ifdef DEBUG

	int64 beTime = CUtil::GetNowSecond();

#endif

//	if(message->GetGroup() >= 0 && message->GetChannelID() < 0)
//	{
//		message->SetChannelID(m_c->GetGroupSession()->GetChannelID(message->GetGroup(), message->GetGroupKey()));
//	}
//	else if(message->GetGroup() < 0 && message->GetChannelID() < 0)
//	{
//		return;
//	}

//	if(message->GetLoopIndex() >= 0)
//	{
//		CEpollMain *  tMain = m_c->GetEpollMgr();
//		if(!tMain)
//			return;
//		CEpollLoop * tLoop = tMain->GetEpollLoop(message->GetLoopIndex());
//		if(!tLoop)
//			return;
//		DispatchBase * tDispatch = tLoop->GetDispatch();
//		if(!tDispatch)
//			return;
//
//		tDispatch->AddSendMessage(message);
//	}
//	else
//	{
//		CEpollMain *  tMain = m_c->GetEpollMgr();
//		if(!tMain)
//			return;
//		CEpollLoop * tLoop = tMain->GetEpollLoop(tMain->GetEpollIndex(message->GetChannelID()));
//		if(!tLoop)
//			return;
//		DispatchBase * tDispatch = tLoop->GetDispatch();
//		if(!tDispatch)
//			return;
//
//		message->SetLoopIndex(tLoop->GetIndex());
//		tDispatch->AddSendMessage(message);
//	}
	m_c->GetDispatch()->GetDispatch(channel)->AddSendMessage(message);

#ifdef DEBUG

	int64 endTime = CUtil::GetNowSecond();
	if(endTime - beTime > 10)
	{
		LOG_BASE(FILEINFO, "============================send message [%d] tim [%lld]e to more==================", message->GetMessageID(), endTime - beTime);
	}

#endif

}

void Message_Facade::UnBindGroupChannel(int64 key, int group)
{
	m_c->GetGroupSession()->UnBindSession(key, group);
}

int Message_Facade::BindGroupChannel(int64 key, int group)
{
	return m_c->GetGroupSession()->BindSession(key, group);
}

void Message_Facade::CloseChannel(int sock)
{
	if(sock < 0)
		return;

//	close(sock);
	LOG_BASE(FILEINFO, "message facade close session[%d] ", sock);
	m_c->GetEpollMgr()->DeleteChannel(sock, 0);
}

void Message_Facade::Wait()
{
	m_e.WaitForSingleEvent();
}

void Message_Facade::UnWait()
{
	m_e.SetEvent();
}

void Message_Facade::Stop()
{
	m_c->Stop();
	if(m_c != 0)
	{
		delete m_c;
		m_c = 0;
	}
}

}
