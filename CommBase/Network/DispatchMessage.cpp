/*
 * DispatchMessage.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "DispatchMessage.h"
#include "NetWorkConfig.h"
#include "MessageHandler.h"
#include "Service_Handler.h"
#include "../Log/Logger.h"
#include "Context.h"
#include "EpollMain.h"
#include "../util.h"

namespace CommBaseOut
{


DispatchBase::DispatchBase(Context *c):m_c(c),m_index(-1),m_token(1),m_timeNow(0)
{
}
DispatchBase::~DispatchBase()
{

}

void DispatchBase::AddMessage(Safe_Smart_Ptr<Message> & message)
{
	GUARD(CSimLock, obj, &m_queLock);
	if(m_deque.size() > 10000)
	{
		LOG_BASE(FILEINFO, "message deque size beyond 10000");
	}

	m_deque.push_back(message);
}

void DispatchBase::AddMessage(deque<Safe_Smart_Ptr<Message> > & message)
{
	GUARD(CSimLock, obj, &m_queLock);
	if(m_deque.size() > 10000)
	{
		LOG_BASE(FILEINFO, "recv message deque size beyond 10000");
	}

	m_deque.insert(m_deque.end(), message.begin(), message.end());
}

void DispatchBase::AddSendMessage(Safe_Smart_Ptr<Message> & message)
{
	int64 bnow = CUtil::GetNowMicrosecod();

//	if(m_c->GetDispatch()->GetBlockThread() <= 1)
	{
		GUARD(CSimLock, obj, &m_sendLock);

		int64 gnow = CUtil::GetNowMicrosecod();

		if(gnow - bnow >= 100 * 1000)
			LOG_BASE(FILEINFO, "add send message but lock too more time[%lld]", gnow - bnow);

		map<int, deque<Safe_Smart_Ptr<Message> > >::iterator it = m_sendDeque.find(message->GetChannelID());
		if(it == m_sendDeque.end())
		{
			deque<Safe_Smart_Ptr<Message> > tDeque;

			tDeque.push_back(message);
			m_sendDeque[message->GetChannelID()] = tDeque;
		}
		else
		{
			it->second.push_back(message);
		}
		obj.UnLock();

		if(message->GetMessageType() == Request)
		{
			int64 hnow = CUtil::GetNowMicrosecod();

			AddAckTimeOut(message);

			int64 fnow = CUtil::GetNowMicrosecod();

			if(fnow - hnow >= 100 * 1000)
				LOG_BASE(FILEINFO, " add Ack timeout too more time[%lld]", fnow - hnow);
		}
	}
//	else
//	{
//		if(message->GetMessageType() == Request)
//		{
//			int64 hnow = CUtil::GetNowMicrosecod();
//
//			AddAckTimeOut(message);
//
//			int64 fnow = CUtil::GetNowMicrosecod();
//
//			if(fnow - hnow >= 100 * 1000)
//				LOG_BASE(FILEINFO, " add Ack timeout too more time[%lld]", fnow - hnow);
//		}
//
//		m_c->GetEpollMgr()->GetEpollLoop(message->GetChannelID())->GetSessionMgr()->AddMessage(message);
//	}

	int64 enow = CUtil::GetNowMicrosecod();

	if(enow - bnow >= 100 * 1000)
		LOG_BASE(FILEINFO, " add send message too more time[%lld]", enow - bnow);
}

int DispatchBase::AddAckTimeOut(Safe_Smart_Ptr<Message> &message)
{
	Safe_Smart_Ptr<AckItem> item = NEW AckItem();

	DWORD64 timeout = CUtil::GetNowSecond() + message->GetTimeout() * 1000;

	item->time = timeout;
	item->req = message;

	GUARD(CSimLock, obj, &m_timeLock);
	if(m_timeOut.size() > 10000)
	{
		LOG_BASE(FILEINFO, "ack message deque size beyond 10000");
	}

	item->token = GetToken();
	m_timeOut[item->token] = item;
	obj.UnLock();

	message->SetReqID(item->token);

	return eNetSuccess;
}

int DispatchBase::DelAckTimeOut(Safe_Smart_Ptr<Message> &message)
{
	GUARD(CSimLock, obj, &m_timeLock);
	map<int, Safe_Smart_Ptr<AckItem> >::iterator it = m_timeOut.find(message->GetReqID());
	if(it != m_timeOut.end())
	{
		message->SetRequest(it->second->req);
		m_timeOut.erase(it++);
	}
	else
	{
		return eTokenErr;
	}

	return eNetSuccess;
}

void DispatchBase::Dispatch(Safe_Smart_Ptr<Message> & message)
{
	if(message->GetMessageTime() > 0)
	{
//		printf("\n dispatch message  session[ msgid = %d] recv time = %lld bbbbbbbbbbbbbbbbbbbbb\n", message->GetMessageID(), CUtil::GetNowSecond() - message->GetMessageTime());
	}
	switch(message->GetMessageType())
	{
	case SimpleMessage:
		{
			Message_Handler* handler = m_c->GetHandlerMgr()->GetMessageHandler(m_c->GetConfig()->local_type_, message->GetMessageID());
			if(handler == 0)
			{
				LOG_BASE(FILEINFO, "dispatching simple message but message id[%d] is not existed", message->GetMessageID());

				return;
			}

#ifdef DEBUG
			int64 beTime = CUtil::GetNowSecond();
#endif

			handler->Handle_Message(message);

#ifdef DEBUG
			int64 endTime = CUtil::GetNowSecond();
			if(endTime - beTime > 500)
			{
//									printf("\n------------------ despatch simple message[id=%d , len=%d] time[%lld] too more-----------------\n", (*it)->GetMessageID(), (*it)->GetLength(),endTime - beTime);
				LOG_BASE(FILEINFO, "------------------ despatch simple message[id=%d , len=%d] time[%lld] too more-----------------", message->GetMessageID(), message->GetLength(),endTime - beTime);
			}
#endif

			break;
		}
	case Request:
		{
			Request_Handler* handler = m_c->GetHandlerMgr()->GetRequestHandler(m_c->GetConfig()->local_type_, message->GetMessageID());
			if(handler == 0)
			{
				LOG_BASE(FILEINFO, "dispatching Request message but message id[%d] is not existed", message->GetMessageID());

				return;
			}

#ifdef DEBUG
			int64 beTime = CUtil::GetNowSecond();
#endif

			handler->Handle_Request(message);

#ifdef DEBUG
			int64 endTime = CUtil::GetNowSecond();
			if(endTime - beTime > 500)
			{
//									printf("\n------------------ despatch request message[id=%d , len=%d] time[%lld] too more-----------------\n", (*it)->GetMessageID(), (*it)->GetLength(),endTime - beTime);
				LOG_BASE(FILEINFO, "------------------ despatch request message[id=%d , len=%d] time[%lld] too more-----------------", message->GetMessageID(), message->GetLength(),endTime - beTime);
			}
#endif

			break;
		}
	case Ack:
		{
			if(message->GetErrno() != eReqTimeOut)
			{
				if(0 != DelAckTimeOut(message))
				{
	//				LOG_BASE(FILEINFO,"request already ack but ack timeout is null");

					return;
				}
			}

			Ack_Handler* handler = m_c->GetHandlerMgr()->GetAckHandler(m_c->GetConfig()->local_type_, message->GetMessageID());
			if(handler == 0)
			{
				return;
			}

#ifdef DEBUG
			int64 beTime = CUtil::GetNowSecond();
#endif

			handler->Handle_Ack(message);

#ifdef DEBUG
			int64 endTime = CUtil::GetNowSecond();
			if(endTime - beTime > 500)
			{
				LOG_BASE(FILEINFO, "------------------ despatch Ack message[id=%d , reqid=%d, len=%d] time[%lld] too more-----------------", message->GetMessageID(), message->GetReqID(), message->GetLength(),endTime - beTime);
			}
#endif

			break;
		}
	case SystemMessage:
		{
			SystemMessageCon connContent;
			Safe_Smart_Ptr<Inet_Addr> addr = 0;

			CUtil::SafeMemmove(&connContent, sizeof(SystemMessageCon), message->GetContent(), message->GetLength());
			addr = NEW Inet_Addr(connContent.ip, connContent.port);

			switch(message->GetMessageID())
			{
			case eConnFailMessage:
				{
					m_c->GetServiceHandler()->on_connect_failed(connContent.channel, connContent.lid, connContent.ltype, connContent.rid, connContent.rtype, connContent.err, addr);

					break;
				}
			case eConnErrMessage:
				{
					m_c->GetServiceHandler()->on_channel_error(connContent.channel, connContent.lid, connContent.ltype, connContent.rid, connContent.rtype, connContent.err, addr);

					break;
				}
			case eConnBuildSuccess:
				{
					m_c->GetServiceHandler()->on_new_channel_build(connContent.channel, connContent.lid, connContent.ltype, connContent.rid, connContent.rtype, addr);

					break;
				}
			}
			break;
		}
	default:
		{
			LOG_BASE(FILEINFO, "DispatchMessage::svr unknown message type");

			return;
		}
	}
}

DispatchMessage::DispatchMessage(Context *c):DispatchBase(c)
{

}

DispatchMessage::~DispatchMessage()
{
	if(!m_flag)
		End();
}

//void DispatchMessage::AddSendMessage(Safe_Smart_Ptr<Message> & message)
//{
//	GUARD(CSimLock, obj, &m_sendLock);
//
//	map<int, deque<Safe_Smart_Ptr<Message> > >::iterator it = m_sendDeque.find(message->GetChannelID());
//	if(it == m_sendDeque.end())
//	{
//		deque<Safe_Smart_Ptr<Message> > tDeque;
//
//		tDeque.push_back(message);
//		m_sendDeque[message->GetChannelID()] = tDeque;
//	}
//	else
//	{
//		it->second.push_back(message);
//	}
//}

int DispatchMessage::svr()
{
	while(!m_flag)
	{
		int64 bnow = 0;
		int64 enow = 0;
		try
		{
			deque<Safe_Smart_Ptr<Message> > tDeque;
			bnow = CUtil::GetNowMicrosecod();

			if((bnow / 1000) - m_timeNow > 1000)
			{
				deque<Safe_Smart_Ptr<Message> > tTimeOut;
				{
					GUARD(CSimLock, obj, &m_timeLock);
					map<int, Safe_Smart_Ptr<AckItem> >::iterator it = m_timeOut.begin();
					for(; it!=m_timeOut.end(); )
					{
						if((DWORD64)(bnow/1000) >= it->second->time)
						{
							Safe_Smart_Ptr<Message> timeOutMe = NEW Message();
							packetHeader head;

							timeOutMe->SetErrno(eReqTimeOut);
							head.messageID = it->second->req->GetMessageID();
							head.messageType = Ack;
							head.remoteType = it->second->req->GetRemoteType();
							head.remoteID = it->second->req->GetRemoteID();
							timeOutMe->SetRequest(it->second->req);
							timeOutMe->SetHead(head);
							timeOutMe->SetChannelID(it->second->req->GetChannelID());
							timeOutMe->SetLoopIndex(it->second->req->GetLoopIndex());

//							Dispatch(timeOutMe);
							tTimeOut.push_back(timeOutMe);
							m_timeOut.erase(it++);

							continue;
						}

						++it;
					}
				}

				deque<Safe_Smart_Ptr<Message> >::iterator itTimeOut = tTimeOut.begin();
				for(; itTimeOut!=tTimeOut.end(); ++itTimeOut)
				{
					Dispatch(*itTimeOut);
				}

				m_timeNow = bnow / 1000;
			}

			{
				GUARD(CSimLock, obj, &m_queLock);
				if(m_deque.size() > 0)
				{
					tDeque.insert(tDeque.end(), m_deque.begin(), m_deque.end());
					m_deque.clear();
//					deque<Safe_Smart_Ptr<Message> >().swap(m_deque);
					obj.UnLock();

					deque<Safe_Smart_Ptr<Message> >::iterator it = tDeque.begin();
					for(; it!=tDeque.end(); ++it)
					{
						Dispatch(*it);
					}
				}
			}

			enow = CUtil::GetNowMicrosecod();

			if(enow - bnow > 500000 && tDeque.size() > 0)
			{
				LOG_BASE(FILEINFO, "dispatch[index=%d] message[count = %zu] time = %lld", m_index, tDeque.size(), enow - bnow);
//				printf("\n\n &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& dispatch message[count = %d]  time = %lld &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n\n", tDeque.size(), enow - bnow);
			}

			int messagecount = 0;
			{
				map<int, deque<Safe_Smart_Ptr<Message> > > tSendDeque;

				GUARD(CSimLock, objSend, &m_sendLock);
				tSendDeque = m_sendDeque;
				m_sendDeque.clear();
//				map<int, deque<Safe_Smart_Ptr<Message> > >().swap(m_sendDeque);
				objSend.UnLock();

				map<int, deque<Safe_Smart_Ptr<Message> > >::iterator itSend = tSendDeque.begin();
				for(; itSend!=tSendDeque.end(); ++itSend)
				{
					messagecount += itSend->second.size();
					m_c->GetEpollMgr()->GetEpollLoop(itSend->first)->GetSessionMgr()->AddMessage(itSend->first, itSend->second);
				}
			}

			bnow = CUtil::GetNowMicrosecod();
			if(bnow - enow > 500000 && messagecount > 0)
			{
				LOG_BASE(FILEINFO, "dispatch[index=%d] send message[count = %d] time = %lld", m_index, messagecount, bnow - enow);
//				printf("\n\n &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& dispatch send message[count = %d] time = %lld &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n\n", messagecount, enow - bnow);
			}
		}
		catch(exception &e)
		{
			LOG_BASE(FILEINFO, "DispatchMessage::svr error and error msg [%s]", e.what());

			continue;
		}
		catch(...)
		{
			LOG_BASE(FILEINFO, "DispatchMessage::svr unknown error");

			continue;
		}

		CUtil::MSleep(10);
	}

	return eNetSuccess;
}


SingleDispatchThread::SingleDispatchThread(Context *c):DispatchBase(c)
{

}

SingleDispatchThread::~SingleDispatchThread()
{
}

void SingleDispatchThread::MessageFlush()
{
//	m_event.SetEvent();
}

void SingleDispatchThread::ProcessMessage()
{
	int64 bnow = 0;
	int64 enow = 0;

	bnow = CUtil::GetNowMicrosecod();
	deque<Safe_Smart_Ptr<Message> > tmpQue;

	if((bnow / 1000) - m_timeNow > 1000)
	{
		deque<Safe_Smart_Ptr<Message> > tTimeOut;
		{
			GUARD(CSimLock, obj, &m_timeLock);
			map<int, Safe_Smart_Ptr<AckItem> >::iterator it = m_timeOut.begin();
			for(; it!=m_timeOut.end(); )
			{
				if((DWORD64)(bnow/1000) >= it->second->time)
				{
					Safe_Smart_Ptr<Message> timeOutMe = NEW Message();
					packetHeader head;

					timeOutMe->SetErrno(eReqTimeOut);
					head.messageID = it->second->req->GetMessageID();
					head.messageType = Ack;
					head.remoteType = it->second->req->GetRemoteType();
					head.remoteID = it->second->req->GetRemoteID();
					timeOutMe->SetRequest(it->second->req);
					timeOutMe->SetHead(head);
					timeOutMe->SetChannelID(it->second->req->GetChannelID());
					timeOutMe->SetLoopIndex(it->second->req->GetLoopIndex());

//					Dispatch(timeOutMe);
					tTimeOut.push_back(timeOutMe);
					m_timeOut.erase(it++);

					continue;
				}

				++it;
			}
		}

		deque<Safe_Smart_Ptr<Message> >::iterator itTimeOut = tTimeOut.begin();
		for(; itTimeOut!=tTimeOut.end(); ++itTimeOut)
		{
			Dispatch(*itTimeOut);
		}

		m_timeNow = bnow / 1000;
	}

	GUARD(CSimLock, obj, &m_queLock);
	if(m_deque.size() <= 0)
	{
		return;
	}

	tmpQue.insert(tmpQue.end(), m_deque.begin(), m_deque.end());
	m_deque.clear();
//	deque<Safe_Smart_Ptr<Message> >().swap(m_deque);
	obj.UnLock();

	if(tmpQue.size() >= 10000)
	{
		LOG_BASE(FILEINFO, "SingleDispatchThread::svr MessageQueue too more");
	}

	deque<Safe_Smart_Ptr<Message> >::iterator it = tmpQue.begin();
	for(; it!=tmpQue.end(); ++it)
	{
		try
		{
			Dispatch(*it);
		}
		catch(exception &e)
		{
			LOG_BASE(FILEINFO, "SingleDispatchThread::svr error and error msg [%s]", e.what());

			continue;
		}
		catch(...)
		{
			LOG_BASE(FILEINFO, "SingleDispatchThread::svr error and error");

			continue;
		}
	}

	enow = CUtil::GetNowMicrosecod();

	if(enow - bnow > 500 * 1000 && tmpQue.size() > 0)
	{
		LOG_BASE(FILEINFO, "dispatch[index=%d] message[count = %zu] time = %lld", m_index, tmpQue.size(), enow - bnow);
//		printf("\ndispatch[index=%d] message[count = %d] time = %lld\n", m_index, tmpQue.size(), enow - bnow);
	}
}

int SingleDispatchThread::svr()
{
	while(!m_flag)
	{
		int64 bnow = 0;
		int64 enow = 0;

		try
		{
//			m_event.WaitForSingleEvent();

			CUtil::MSleep(10);
			bnow = CUtil::GetNowMicrosecod();
			GUARD(CSimLock, obj, &m_sendLock);
			if(m_sendDeque.size() <= 0)
			{
				continue;
			}

			map<int, deque<Safe_Smart_Ptr<Message> > > tDeque;

			tDeque = m_sendDeque;
			m_sendDeque.clear();
//			map<int, deque<Safe_Smart_Ptr<Message> > >().swap(m_sendDeque);
//			m_sendDeque.swap(map<int, deque<Safe_Smart_Ptr<Message> > >());
			obj.UnLock();

			map<int, deque<Safe_Smart_Ptr<Message> > >::iterator itSend = tDeque.begin();
			for(; itSend!=tDeque.end(); ++itSend)
			{
				m_c->GetEpollMgr()->GetEpollLoop(itSend->first)->GetSessionMgr()->AddMessage(itSend->first, itSend->second);
			}

			enow = CUtil::GetNowMicrosecod();

			if(enow - bnow > 100 * 1000 && tDeque.size() > 0)
				LOG_BASE(FILEINFO, "dispatch[index=%d] message[count = %zu] time = %lld", m_index, tDeque.size(), enow - bnow);
		}
		catch(exception &e)
		{
			LOG_BASE(FILEINFO, "SingleDispatchThread::svr error and error msg [%s]", e.what());

			continue;
		}
		catch(...)
		{
			LOG_BASE(FILEINFO, "SingleDispatchThread::svr unknown error");

			continue;
		}
	}

	return eNetSuccess;
}


int DispatchMgr::Init(int count)
{
	if( count <= 0 )
		count = 1;

	m_blockThread = count;
	for( int i=0; i<count; ++i )
	{
		DispatchBase *  tBase;

		if( count > 1 )
		{
			tBase = NEW DispatchMessage( m_c );

			if( tBase->Start( 1 ) )
			{
				return eInitDispatchErr;
			}
		}
		else
		{
			tBase = NEW SingleDispatchThread( m_c );

			if( tBase->Start( 1 ) )
			{
				return eInitDispatchErr;
			}
		}

		tBase->SetIndex(i);

		m_allDispatch.push_back(tBase);
	}

	return eNetSuccess;
}

DispatchBase *  DispatchMgr::GetDispatch()
{
	int tIndex = m_index++;
	if(m_index >= m_blockThread)
		m_index = 0;

	return m_allDispatch[tIndex];
}

void DispatchMgr::DeleteAll()
{
	vector<DispatchBase *  >::iterator it = m_allDispatch.begin();
	for(; it!=m_allDispatch.end(); ++it)
	{
		(*it)->Close();
		delete (*it);
		(*it) = 0;
	}

	m_allDispatch.clear();
}

}
