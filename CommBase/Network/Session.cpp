/*
 * Session.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "Session.h"
#include "EpollMain.h"
#include "Context.h"
#include "../util.h"
#include "../Log/Logger.h"
#include "MessageManager.h"
#include "DispatchMessage.h"
#include "GroupSession.h"

namespace CommBaseOut
{

Session::Session(Context *c, CSessionMgr *s, int sock, int flag, struct sockaddr_in &addr, bool isSecurity):m_c(c),m_remoteID(0),
		m_remoteType(0),m_accSock(sock),m_lastPacketTime(0), m_flag(flag),m_bufRecv(0),dwRecvPos(0),m_s(s), m_security(isSecurity),bufSend(0),m_groupID(-1)
#ifdef USE_PACKAGE_COUNT
,m_recvCount(0),m_sendCount(0),m_recvTime(0),m_sendTime(0)
#endif
{
	m_addr = NEW Inet_Addr(&addr);
	m_bufRecv = NEW_BASE(char, MAX_RECV_MSG_SIZE);// char[MAX_RECV_MSG_SIZE];
	bufSend = NEW CCircleBuff(MAX_NET_MSG_SIZE[eClientBuf]);
}

Session::~Session()
{
	if(m_accSock >= 0)
	{
		m_accSock = -1;
	}

	DELETE_BASE(m_bufRecv, eMemoryArray);
	m_bufRecv = 0;

	delete bufSend;
	bufSend = 0;
}

char * Session::GetRecvBuf()
{
	return m_bufRecv;
}

DWORD Session::GetRecvPos()
{
	return dwRecvPos;
}

int Session::AddMessage(Safe_Smart_Ptr<Message> &message)
{
	try
	{
		if(message->GetMessageType() == ConnMessage)
		{
			int res = -1;
			if((res=m_s->ConnectSuccess(message)) != 0 && res != eGroupContinue)
			{
				m_c->GetEpollMgr()->DeleteChannel(message->GetChannelID(), res);
//				printf("\n---------------connectsuccess close session = %d-----------\n", message->GetChannelID());

				return -1;
			}

			if(res != eGroupContinue)
			{
				if(message->GetGroup() >= 0)
					m_c->SystemErr(message->GetGroup(), message->GetLocalID(), message->GetLocalType(),
							message->GetRemoteID(), message->GetRemoteType(), 0, message->GetAddr()->GetIPToString(),message->GetAddr()->GetPort());
				else
					m_c->SystemErr(message->GetChannelID(), message->GetLocalID(), message->GetLocalType(),
							message->GetRemoteID(), message->GetRemoteType(), 0, message->GetAddr()->GetIPToString(),message->GetAddr()->GetPort());
			}

			return 1;
		}

//		if(message->GetMessageType() == Ack)
//		{
//			if(0 != m_c->GetAckTimeout()->DelAckTimeOut(message))
//			{
////				LOG_BASE(FILEINFO,"request already ack but ack timeout is null");
//
//				return -1;
//			}
//		}

//		m_c->GetDispatch()->GetDispatch(message->GetChannelID())->AddMessage(message);
//		m_c->GetEpollMgr()->GetEpollLoop((m_flag >> 8) & 0xff)->GetDispatch()->AddMessage(message);
	}
	catch(exception &e)
	{
		LOG_BASE(FILEINFO, "add message error[errmsg = %s]", e.what());
		return -1;
	}
	catch(...)
	{
		LOG_BASE(FILEINFO, "add message unknown error");
		return -1;
	}

	return 0;
}

int Session::Recv(deque<Safe_Smart_Ptr<Message> > &vec)
{
	int ret = 0;
	packetHeader head;
	const char *tmpHead = m_bufRecv;
	int nLeftSize = dwRecvPos;

	while(1)
	{
		if(nLeftSize < HEADER_LENGTH)
		{
			dwRecvPos = nLeftSize;
			if(0 != CUtil::SafeMemmove(m_bufRecv, MAX_RECV_MSG_SIZE - dwRecvPos, tmpHead, dwRecvPos))
			{
				dwRecvPos = 0;
				LOG_BASE(FILEINFO, "the rest is less than header and recv copy error ,so clear package content");
			}

			break;
		}

		int moveRes = CUtil::SafeMemmove(&head, HEADER_LENGTH, tmpHead, HEADER_LENGTH);
		if (0 != moveRes)
		{
			dwRecvPos = nLeftSize;
			if(0 != CUtil::SafeMemmove(m_bufRecv, MAX_RECV_MSG_SIZE - dwRecvPos, tmpHead, nLeftSize))
			{
				dwRecvPos = 0;
				LOG_BASE(FILEINFO, "copy header and recv copy error, so clear package content ");

				break;
			}

			break;
		}

		head.toBigEndian();
//		head.toSmallEndian();
		if(head.remoteID < 0 || head.length < 0 || head.remoteType >= eMax || head.localType >= eMax || head.messageType > SystemMessage)
		{
			LOG_BASE(FILEINFO, "serialize header but header content error, so clear package content  ip[%s]", m_addr->GetIPToChar());
			if(m_remoteType == eClient)
				return eSocketClose;

			dwRecvPos = 0;

			break;
		}

		if(MAX_MSG_PACKET_SIZE < head.length  + HEADER_LENGTH)
		{
			LOG_BASE(FILEINFO, "recv but length more than package length   ip[%s]", m_addr->GetIPToChar());
			if(m_remoteType == eClient)
				return eSocketClose;

			dwRecvPos = 0;

			break;
		}

		if (nLeftSize < HEADER_LENGTH + head.length)
		{
			dwRecvPos = nLeftSize;
			if(0 != CUtil::SafeMemmove(m_bufRecv, MAX_RECV_MSG_SIZE - dwRecvPos, tmpHead, nLeftSize))
			{
				dwRecvPos = 0;
				LOG_BASE(FILEINFO, "the rest is not all package and recv copy error, so clear package content ");
			}

			break;
		}

#ifdef USE_PACKAGE_COUNT
		m_recvCount++;
#endif

		Safe_Smart_Ptr<Message> recvMessage = NEW Message(m_remoteID, m_remoteType);

		if(recvMessage && recvMessage->GetContent())
		{
			recvMessage->SetHead(head);
			if(head.remoteID == m_c->GetConfig()->local_id_ &&  head.remoteType == m_c->GetConfig()->local_type_ && recvMessage->GetSecurity() == m_security)
			{
				recvMessage->SetContent(tmpHead+HEADER_LENGTH, head.length);
				recvMessage->SetAddr(m_addr);
				recvMessage->SetChannelID(m_accSock);
				recvMessage->SetGroup(m_groupID);
				recvMessage->SetLoopIndex((m_flag >> 8) & 0xff);
				if(recvMessage->UnEncryptMessage())
				{
					if(recvMessage->GetMessageTime() > 0)
					{
	//					printf("\n recv message  session[ msgid = %d] recv time = %lld bbbbbbbbbbbbbbbbbbbbb\n", recvMessage->GetMessageID(), CUtil::GetNowSecond() - recvMessage->GetMessageTime());
					}

					ret = AddMessage(recvMessage);
					if(ret < 0)
					{
						LOG_BASE(FILEINFO, "recv but add message error and throw out pakcage");
					}
					else if(ret == 0)
					{
						vec.push_back(recvMessage);
					}
				}
			}
		}

		tmpHead += (HEADER_LENGTH + head.length);
		nLeftSize -= (HEADER_LENGTH + head.length);

		if(nLeftSize <= 0)
		{
			dwRecvPos = 0;
			break;
		}
	}

	return ret;
}

int Session::Recv()
{
	int ret = 0;
	deque<Safe_Smart_Ptr<Message> > vec;

	while(1)
	{
		int nRet = recv(m_accSock, m_bufRecv + dwRecvPos, MAX_RECV_MSG_SIZE - dwRecvPos, 0);
		if(nRet == 0)
		{
			return eSocketClose;
		}
		else if(nRet < 0)
		{
			if(errno == EAGAIN)
			{
				break;
			}
			else
			{
				char errorMsg[1024] = {0};

				perror(errorMsg);
				LOG_BASE(FILEINFO, "recv find net error[%s] and recv end", errorMsg);

				ret = eNetError;

				break;
			}
		}

		dwRecvPos += nRet;

		const char *tmpHead = m_bufRecv;

//		if(dwRecvPos == 23 && strcmp(tmpHead, "<policy-file-request/>") == 0)
//		{
//			char rbuf[512]="<?xml version=\"1.0\"?><cross-domain-policy><site-control permitted-cross-domain-policies=\"all\"/><allow-access-from domain=\"*\" to-ports=\"*\"/></cross-domain-policy>\0";
//			LOG_BASE(FILEINFO, "\n ++++++++++++  recv http[%s] socket[%d] +++++++++++++++++++\n", rbuf, m_accSock);
//			Write(rbuf, strlen(rbuf) + 1);
//
//			return eNetSuccess;
//		}

		if(dwRecvPos < (DWORD)HEADER_LENGTH )
		{
			continue;
		}

		ret = Recv(vec);
		if(ret == eSocketClose)
		{
			return ret;
		}
	}

	ret = Recv(vec);
	if(ret == eSocketClose)
	{
		return ret;
	}

	m_c->GetDispatch()->GetDispatch(m_accSock)->AddMessage(vec);

#ifdef USE_PACKAGE_COUNT

	if(m_recvTime <= 0)
	{
		m_recvTime = CUtil::GetNowSecond();
	}
	else
	{
		int64 nowTime = CUtil::GetNowSecond();

		if(nowTime - m_recvTime > 20 * 1000)
		{
			int tCount = m_recvCount / 20;
			if(tCount > 60)
			{
				if(m_remoteType <= eClient || m_remoteType >= eMax)
				{
					LOG_BASE(FILEINFO, "session send to many package[%d] and close session", tCount);

					return eSocketClose;
				}
			}

			m_recvCount = 0;
			m_recvTime = nowTime;
		}
	}

#endif

	return eNetSuccess;
}

int Session::Write(Safe_Smart_Ptr<Message> &message)
{
	int res = -1;
	packetHeader head;

	if(message->GetSecurity() != m_security)
	{
		message->SetSecurity(m_security);
	}

	message->EncryptMessage();
	message->GetHead(&head);

//	printf("\n++++++++++++++++write2 begin session = %x+++++++++++++++++++++++++++\n", this);
	GUARD(CSimLock, obj, &m_bufLock);
	if((res = bufSend->WriteSend(message->GetContent(), &head)) != 0)
	{
		LOG_BASE(FILEINFO, "send writebuff error and session[rid=%d, rtype=%d, messageid=%d]", m_remoteID, m_remoteType, head.messageID);

		return res;
	}
	obj.UnLock();

//	printf("\n++++++++++++++++write2 end session = %x+++++++++++++++++++++++++++\n", this);

	struct epoll_event ev;
	bzero(&ev, sizeof(ev));
	ev.events = EPOLLIN | EPOLLET | EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
//	ev.data.fd = m_accSock;
	ev.data.ptr = (void *)m_s->GetEventPtr(m_accSock);
	if (m_c->GetEpollMgr()->GetEpollLoop(m_accSock)->EpollCtl(EPOLL_CTL_MOD, m_accSock, &ev) != 0)
	{
		LOG_BASE(FILEINFO, "send get epoll error");

		return eCtlEpollErr;
	}

	return eNetSuccess;
}

int Session::Write(char *buf, int len)
{
	int res = -1;

//	printf("\n++++++++++++++++write1 begin session = %x+++++++++++++++++++++++++++\n", this);
	GUARD(CSimLock, obj, &m_bufLock);
	if((res = bufSend->WriteBuffer(buf, len)) != 0)
	{
		LOG_BASE(FILEINFO, "send writebuff error and session[rid=%d, rtype=%d]", m_remoteID, m_remoteType);

		return res;
	}
	obj.UnLock();

//	printf("\n++++++++++++++++write1 end session = %x+++++++++++++++++++++++++++\n", this);

	struct epoll_event ev;
	bzero(&ev, sizeof(ev));
	ev.events = EPOLLIN | EPOLLET | EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
//	ev.data.fd = m_accSock;
	ev.data.ptr = (void *)m_s->GetEventPtr(m_accSock);
	if (m_c->GetEpollMgr()->GetEpollLoop(m_accSock)->EpollCtl(EPOLL_CTL_MOD, m_accSock, &ev) != 0)
	{
		LOG_BASE(FILEINFO, "send get epoll error");

		return eCtlEpollErr;
	}

	return eNetSuccess;
}

int Session::Write(char *content, packetHeader* head)
{
	int res = -1;

//	printf("\n++++++++++++++++write2 begin session = %x+++++++++++++++++++++++++++\n", this);
	GUARD(CSimLock, obj, &m_bufLock);
	if((res = bufSend->WriteSend(content, head)) != 0)
	{
		if(m_remoteType == eClient)
			head->toBigEndianEx();

		LOG_BASE(FILEINFO, "send writebuff error[return[%d]] and session[socket=%d,rid=%d, rtype=%d, messageid=%d, length=%d]", res, m_accSock, m_remoteID, m_remoteType, head->messageID, head->length);

		return res;
	}
	obj.UnLock();

//	printf("\n++++++++++++++++write2 end session = %x+++++++++++++++++++++++++++\n", this);

	struct epoll_event ev;
	bzero(&ev, sizeof(ev));
	ev.events = EPOLLIN | EPOLLET | EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
//	ev.data.fd = m_accSock;
	ev.data.ptr = (void *)m_s->GetEventPtr(m_accSock);
	if (m_c->GetEpollMgr()->GetEpollLoop(m_accSock)->EpollCtl(EPOLL_CTL_MOD, m_accSock, &ev) != 0)
	{
		LOG_BASE(FILEINFO, "send get epoll error socket[%d]", m_accSock);

		return eCtlEpollErr;
	}

	return eNetSuccess;
}

int Session::Send()
{
	int64 belocktime =  CUtil::GetNowMicrosecod();

	GUARD(CSimLock, obj, &m_bufLock);

	int64 endlocktime =  CUtil::GetNowMicrosecod();
	if(endlocktime - belocktime >= 100 * 1000)
		LOG_BASE(FILEINFO, "send message but lock too more time[%lld]", endlocktime - belocktime);

	if(bufSend->IsEmpty())
	{
		return eNetSuccess;
	}

	const char *bufRead = NULL;
	int nBuffLen = 0;
	if (bufSend->ReadSendBuff(bufRead, nBuffLen) != 0)
	{
		return eNetSuccess;
	}

	int nSendTotal = 0;
//	printf("\n----------------%d session=%x send=%x readindex=%d writeindex=%d lastindex = %d------------------------\n", m_accSock, this, bufSend,bufSend->GetReadIndex(), bufSend->GetWriteIndex(), bufSend->GetLastIndex());
	while(1)
	{
		int nRet = send(m_accSock, bufRead + nSendTotal, nBuffLen - nSendTotal, MSG_NOSIGNAL);

		if(nRet == 0)
		{
			LOG_BASE(FILEINFO, "send socket error");

			return eSocketClose;
		}
		else if(nRet < 0)
		{
			if(errno == EAGAIN)
			{
				break;
			}
			else
			{
				LOG_BASE(FILEINFO, "socket send error");

				return eSockSendErr;
			}
		}

		nSendTotal += nRet;
		if(nSendTotal >= nBuffLen)
		{
			break;
		}
	}
//	printf("\n----------------%d session=%x send=%x  send size=%d------------------------\n", m_accSock, this, bufSend, nSendTotal);
	bufSend->HadRead(nSendTotal);

	int64 endtime = CUtil::GetNowMicrosecod();
	if(endtime - belocktime >= 100 * 1000)
		LOG_BASE(FILEINFO, "send message from tcp too more time[%lld]", endtime - belocktime);

	return eNetSuccess;
}

void Session::AddPos(DWORD len)
{
	dwRecvPos += len;
}

void Session::SetPos(DWORD len)
{
	dwRecvPos = len;
}

void Session::SetRemoteID(short int id)
{
	m_remoteID = id;
}

void Session::SetRemoteType(unsigned char type)
{
	m_remoteType = type;
}

int Session::GetFlag()
{
	return m_flag;
}

CSessionMgr::CSessionMgr(Context *c):m_bTime(CUtil::GetNowMicrosecod()),m_tick(0),m_epollPtr(0),m_c(c)
{
	 m_epollPtr = new EpollEventPtr[MAX_EPOLL_EVENT];
}

CSessionMgr::~CSessionMgr()
{
	m_session.clear();
	m_timeOutSe.clear();

	if(m_epollPtr)
	{
		delete[] m_epollPtr;
		m_epollPtr = 0;
	}
}

EpollEventPtr * CSessionMgr::AddTimerSessionEx(int sock, struct sockaddr_in &addr, ChannelConfig * config, WORD index, unsigned char type)
{
	if(MAX_EPOLL_EVENT <= sock)
	{
		return 0;
	}

	int key = index;
	Safe_Smart_Ptr<Session> se;

	key = ((key << 8) | type);
	if(config->channel_keep_time_ > 0)
	{
		GUARD_WRITE(CRWLock, obj, &m_timeSeLock);
		map<int, Safe_Smart_Ptr<Session> >::iterator it = m_timeOutSe.find(sock);
		if(it != m_timeOutSe.end())
		{
			LOG_BASE(FILEINFO, "add session same timer so error");

			return 0;
		}

		se = NEW Session(m_c, this, sock, key, addr, config->security_);
		m_timeOutSe[sock] = se;
		obj.UnLock();

		m_timeout[sock] = config->channel_keep_time_ + CUtil::GetNowSecond();
		m_epollPtr[sock].se = se;

		return &m_epollPtr[sock];
	}
	else
	{
		return 0;
	}

	return 0;
}

int CSessionMgr::ConnectSuccess(Safe_Smart_Ptr<Message> &message)
{
	int successRet = 0;

	GUARD_WRITE(CRWLock, obj, &m_timeSeLock);
	map<int, Safe_Smart_Ptr<Session> >::iterator it = m_timeOutSe.find(message->GetChannelID());
	if(it != m_timeOutSe.end())
	{
		if(message->GetMessageID() == C2SMessage)
		{
			if(message->GetLength() != 3)
			{
				LOG_BASE(FILEINFO, "client to server connection packet error");

				return eConMessageErr;
			}

			short int id = 0;
			unsigned char type = 0;

			CUtil::SafeMemmove(&type, 1, message->GetContent(), 1);
			CUtil::SafeMemmove(&id, 2, message->GetContent() + 1, 2);

			if(message->GetRemoteType() <= 0)
				id = ntohs(id);

			if(type >= eMax)
			{
				LOG_BASE(FILEINFO, "client to server connection packet type error");

				return eConMessageErr;
			}
			message->SetRemoteID(id);
			message->SetRemoteType(type);
			it->second->SetRemoteID(id);
			it->second->SetRemoteType(type);
		}
		else if(message->GetMessageID() == S2CMessage)
		{
			if(message->GetLength() != 1)
			{
				LOG_BASE(FILEINFO, "server to client connection packet error");

				return eConMessageErr;
			}

			unsigned char ret = 0;

			CUtil::SafeMemmove(&ret, 1, message->GetContent(), 1);

			if(ret != 0)
			{
				LOG_BASE(FILEINFO, "server to client connection packet return error");

				return eConMessageErr;
			}

			int id = m_c->GetConnConfig(it->second->GetFlag() >> 16)->remote_id_;
			int type = m_c->GetConnConfig(it->second->GetFlag() >> 16)->remote_type_;

			message->SetRemoteID(id);
			message->SetRemoteType(type);
			it->second->SetRemoteID(id);
			it->second->SetRemoteType(type);
		}
		else if(message->GetMessageID() == C2SGroupMessage)
		{
			if(message->GetLength() != 4)
			{
				LOG_BASE(FILEINFO, "client to server group message error");
				return eConMessageErr;
			}

			short int id = 0;
			unsigned char type = 0;
			BYTE groupCount = 0;
			int res = -1;
			bool isSuccess = false;

			CUtil::SafeMemmove(&type, 1, message->GetContent(), 1);
			CUtil::SafeMemmove(&id, 2, message->GetContent() + 1, 2);
			CUtil::SafeMemmove(&groupCount, 1, message->GetContent() + 3, 1);

			if(type >= eMax)
			{
				LOG_BASE(FILEINFO, "client to server group message type error");
				return eConMessageErr;
			}

			message->SetRemoteID(id);
			message->SetRemoteType(type);
			it->second->SetRemoteID(id);
			it->second->SetRemoteType(type);

			if((res = m_c->GetGroupSession()->AddGroupSession(type, id, message->GetChannelID(), groupCount, isSuccess)) < -1)
			{
				LOG_BASE(FILEINFO, "client to server group message and add group error");
				return eConMessageErr;
			}

			if(!isSuccess)
			{
				successRet = eGroupContinue;
			}

			it->second->SetGroupID(res);
			message->SetGroup(res);
		}
		else if(message->GetMessageID() == S2CGroupMessage)
		{
			if(message->GetLength() != 1)
			{
				LOG_BASE(FILEINFO, "server to client group message error");
				return eConMessageErr;
			}

			unsigned char ret = 0;
			int res = -1;
			int type = 0;
			int id = 0;
			int groupCount = 0;
			bool isSuccess = false;

			CUtil::SafeMemmove(&ret, 1, message->GetContent(), 1);

			if(ret != 0)
			{
				LOG_BASE(FILEINFO, "server to client group message return error");
				return eConMessageErr;
			}

			type = m_c->GetConnConfig(it->second->GetFlag() >> 16)->remote_type_;
			id = m_c->GetConnConfig(it->second->GetFlag() >> 16)->remote_id_;
			groupCount = m_c->GetConnConfig(it->second->GetFlag() >> 16)->group_count;

			message->SetRemoteID(id);
			message->SetRemoteType(type);
			it->second->SetRemoteID(id);
			it->second->SetRemoteType(type);

			if((res = m_c->GetGroupSession()->AddGroupSession(type, id, message->GetChannelID(), groupCount, isSuccess)) < 0)
			{
				LOG_BASE(FILEINFO, "server to client group message and add group error");
				return eConMessageErr;
			}

			if(!isSuccess)
			{
				successRet = eGroupContinue;
			}

			it->second->SetGroupID(res);
			message->SetGroup(res);
		}
		else
		{
			LOG_BASE(FILEINFO, "connection packet unknown messageid error");

			return eConMessageErr;
		}

		Safe_Smart_Ptr<Session> se = it->second;
		m_timeOutSe.erase(it);
		obj.UnLock();

		m_timeout.erase(message->GetChannelID());

		GUARD_WRITE(CRWLock, objSe, &m_seLock);
		m_session[se->GetSock()] = se;
		objSe.UnLock();

		if(se->GetRemoteType() != eClient)
		{//非客户端的连接，把发送缓冲调大
			se->ResetSendBuf();
		}

		if(message->GetMessageID() == C2SMessage)
		{
			Safe_Smart_Ptr<Message> messageret = NEW Message();
			AcceptorConfig * config = m_c->GetAccConfig(se->GetFlag() >> 16);

			messageret->SetSecurity(config->security_);
			messageret->SetRemoteID(se->GetRemoteID());
			messageret->SetRemoteType(se->GetRemoteType());
			messageret->SetLocalType(config->local_type_);
			messageret->SetChannelID(message->GetChannelID());
			messageret->SetMessageID(S2CMessage);
			messageret->SetMessageType(ConnMessage);

			char con[8] = {0};
			BYTE conRet = eNetSuccess;

			CUtil::SafeMemmove(con, 8, &conRet, 1);

			messageret->SetContent(con, 1);
			if(Write(messageret))
			{
				LOG_BASE(FILEINFO, "server to client connection packet write buff error");
			}
		}
		else if(message->GetMessageID() == C2SGroupMessage)
		{
			Safe_Smart_Ptr<Message> messageret = NEW Message();
			AcceptorConfig * config = m_c->GetAccConfig(se->GetFlag() >> 16);

			messageret->SetSecurity(config->security_);
			messageret->SetRemoteID(se->GetRemoteID());
			messageret->SetRemoteType(se->GetRemoteType());
			messageret->SetLocalType(config->local_type_);
			messageret->SetChannelID(message->GetChannelID());
			messageret->SetMessageID(S2CGroupMessage);
			messageret->SetMessageType(ConnMessage);

			char con[8] = {0};
			BYTE conRet = eNetSuccess;

			CUtil::SafeMemmove(con, 8, &conRet, 1);

			messageret->SetContent(con, 1);
			if(Write(messageret))
			{
				LOG_BASE(FILEINFO, "server to client connection packet write buff error");
			}
		}
	}
	else
	{
		return eConnPacketErr;
	}

	return successRet;
}

unsigned char CSessionMgr::GetRemoteType(int fd)
{
	if(fd < 0)
	{
		return 0;
	}

	GUARD_READ(CRWLock, obj, &m_seLock);
	map<int, Safe_Smart_Ptr<Session> >::iterator it = m_session.find(fd);
	if(it != m_session.end())
	{
		return it->second->GetRemoteType();
	}
	obj.UnLock();

	GUARD_READ(CRWLock, objTime, &m_timeSeLock);
	map<int, Safe_Smart_Ptr<Session> >::iterator itTime = m_timeOutSe.find(fd);
	if(itTime != m_timeOutSe.end())
	{
		return itTime->second->GetRemoteType();
	}

	return 0;
}

Safe_Smart_Ptr<Inet_Addr> CSessionMgr::GetAddr(int fd)
{
	if(fd < 0)
	{
		return 0;
	}

	GUARD_READ(CRWLock, obj, &m_seLock);
	map<int, Safe_Smart_Ptr<Session> >::iterator it = m_session.find(fd);
	if(it != m_session.end())
	{
		return it->second->GetAddr();
	}
	obj.UnLock();

	GUARD_READ(CRWLock, objTime, &m_timeSeLock);
	map<int, Safe_Smart_Ptr<Session> >::iterator itTime = m_timeOutSe.find(fd);
	if(itTime != m_timeOutSe.end())
	{
		return itTime->second->GetAddr();
	}

	return 0;
}

short int CSessionMgr::GetRemoteID(int fd)
{
	if(fd < 0)
	{
		return 0;
	}

	GUARD_READ(CRWLock, obj, &m_seLock);
	map<int, Safe_Smart_Ptr<Session> >::iterator it = m_session.find(fd);
	if(it != m_session.end())
	{
		return it->second->GetRemoteID();
	}
	obj.UnLock();

	GUARD_READ(CRWLock, objTime, &m_timeSeLock);
	map<int, Safe_Smart_Ptr<Session> >::iterator itTime = m_timeOutSe.find(fd);
	if(itTime != m_timeOutSe.end())
	{
		return itTime->second->GetRemoteID();
	}

	return 0;
}

int CSessionMgr::Write(Safe_Smart_Ptr<Message> &message)
{
	Safe_Smart_Ptr<Session> se;

	GUARD_READ(CRWLock, obj, &m_seLock);
	map<int, Safe_Smart_Ptr<Session> >::iterator it = m_session.find(message->GetChannelID());
	if(it != m_session.end())
	{
		se = it->second;
	}
	obj.UnLock();

	if(!se)
	{
		GUARD_READ(CRWLock, objTime, &m_timeSeLock);
		map<int, Safe_Smart_Ptr<Session> >::iterator itTime = m_timeOutSe.find(message->GetChannelID());
		if(itTime != m_timeOutSe.end())
		{
			se = itTime->second;
		}
	}

	if((bool)se)
	{
		packetHeader head;

		if(message->GetSecurity() != se->GetSecurity())
		{
			message->SetSecurity(se->GetSecurity());
		}

		message->EncryptMessage();
		message->GetHead(&head);

		if(se->Write(message->GetContent(), &head) == eCtlEpollErr)
		{
			m_c->GetEpollMgr()->DeleteChannelEx(message->GetChannelID(), eCtlEpollErr);

			LOG_BASE(FILEINFO, "write message to session[%d] buff error", message->GetChannelID());

			return eCtlEpollErr;
		}
	}

	return eNetSuccess;
}

void CSessionMgr::DeleteAll()
{
	m_session.clear();
	m_timeOutSe.clear();

//	{
//		map<int, EpollEventPtr *>::iterator itPtr = m_epollPtr.begin();
//		for(; itPtr != m_epollPtr.end(); ++itPtr)
//		{
//			delete itPtr->second;
//			itPtr->second = 0;
//		}
//
//		m_epollPtr.clear();
//	}
}

void CSessionMgr::Tick()
{
	int64 nowTime = CUtil::GetNowMicrosecod();
	vector<int> delChannel;

	m_tick += nowTime - m_bTime;

	m_bTime = nowTime;
	if(m_tick >= 1000 * 1000)
	{
		m_tick -= 1000 * 1000;

		map<int, int64>::iterator it = m_timeout.begin();
		for(; it!=m_timeout.end(); )
		{
			if(nowTime >= (it->second * 1000))
			{
				delChannel.push_back(it->first);
				m_timeout.erase(it++);
			}
			else
			{
				++it;
			}
		}

		vector<int>::iterator itDel = delChannel.begin();
		for(; itDel!=delChannel.end(); ++itDel)
		{
			DeleteSession(*itDel, eSessionTimeout);
			LOG_BASE(FILEINFO, "session timeout delete session[%d]", *itDel);
		}
	}
}

void CSessionMgr::DeleteSessionEx(int fd, int err)
{
	bool isDel = false;

#ifdef DEBUG
	int64 beTime = CUtil::GetNowSecond();
#endif

	{
		GUARD_WRITE(CRWLock, objTime, &m_timeSeLock);
		map<int, Safe_Smart_Ptr<Session> >::iterator itTime = m_timeOutSe.find(fd);
		if(itTime != m_timeOutSe.end())
		{
//			m_c->SystemErr(fd, m_c->GetConfig()->local_id_, m_c->GetConfig()->local_type_,
//					itTime->second->GetRemoteID(), itTime->second->GetRemoteType(), err, itTime->second->GetAddr()->GetIPToString(), itTime->second->GetAddr()->GetPort());
			itTime->second->closeSocket();
			isDel = true;
			m_timeOutSe.erase(itTime);
			objTime.UnLock();

			m_timeout.erase(fd);
		}
	}

	if(!isDel)
	{
		GUARD_READ(CRWLock, obj, &m_seLock);
		map<int, Safe_Smart_Ptr<Session> >::iterator it = m_session.find(fd);
		if(it != m_session.end())
		{
			m_c->SystemErr(fd, m_c->GetConfig()->local_id_, m_c->GetConfig()->local_type_,
					it->second->GetRemoteID(), it->second->GetRemoteType(), err, it->second->GetAddr()->GetIPToString(), it->second->GetAddr()->GetPort());
		}
	}

#ifdef DEBUG
	int64 endTime = CUtil::GetNowSecond();
	if(endTime - beTime > 100)
	{
		LOG_BASE(FILEINFO,"1111111111111111111111 delete session[%d] time[%lld] too more11111111111111111", fd, endTime - beTime);
	}
#endif
}

void CSessionMgr::DeleteSession(int fd, int err)
{
	bool isDel = false;

#ifdef DEBUG
	int64 beTime = CUtil::GetNowSecond();
#endif

	{
		GUARD_WRITE(CRWLock, objTime, &m_timeSeLock);
		map<int, Safe_Smart_Ptr<Session> >::iterator itTime = m_timeOutSe.find(fd);
		if(itTime != m_timeOutSe.end())
		{
			itTime->second->closeSocket();
			isDel = true;
			m_timeOutSe.erase(itTime);
			objTime.UnLock();

			m_timeout.erase(fd);
//			printf("\n  --------------- delete timeout socket[%d] -----------------\n", fd);
		}
	}

	if(!isDel)
	{
		GUARD_WRITE(CRWLock, obj, &m_seLock);
		map<int, Safe_Smart_Ptr<Session> >::iterator it = m_session.find(fd);
		if(it != m_session.end())
		{
			if(it->second->GetGroupID() > 0)
				m_c->GetGroupSession()->DeleteGroupSession(it->second->GetGroupID(), fd);
			it->second->closeSocket();
			m_session.erase(it);
//			printf("\n  --------------- delete socket[%d] -----------------\n", fd);
		}
	}

#ifdef DEBUG
	int64 endTime = CUtil::GetNowSecond();
	if(endTime - beTime > 100)
	{
		LOG_BASE(FILEINFO,"1111111111111111111111 delete session[%d] time[%lld] too more11111111111111111", fd, endTime - beTime);
	}
#endif
}

void CSessionMgr::AddMessage(int channel, deque<Safe_Smart_Ptr<Message> > &messageDeque)
{
	Safe_Smart_Ptr<Session> se;
	{
		GUARD_READ(CRWLock, obj, &m_seLock);
		map<int, Safe_Smart_Ptr<Session> >::iterator itTime = m_session.find(channel);
		if(itTime != m_session.end())
		{
			se = itTime->second;
		}
		else
		{
			return;
		}
	}

	deque<Safe_Smart_Ptr<Message> >::iterator itDeque = messageDeque.begin();
	for(; itDeque!=messageDeque.end(); ++itDeque)
	{
//		if((*itDeque)->GetMessageType() == Request)
//		{
//			if(0 != m_c->GetAckTimeout()->AddAckTimeOut(*itDeque))
//			{
//				continue;
//			}
//		}

		packetHeader head;

		if((*itDeque)->GetRemoteID() < 0)
		{
			(*itDeque)->SetRemoteID(se->GetRemoteID());
			(*itDeque)->SetRemoteType(se->GetRemoteType());
		}

		if((*itDeque)->GetSecurity() != se->GetSecurity())
		{
			(*itDeque)->SetSecurity(se->GetSecurity());
		}

		(*itDeque)->EncryptMessage();
		(*itDeque)->GetHead(&head);

		if(se->Write((*itDeque)->GetContent(), &head) == eCtlEpollErr)
		{
			LOG_BASE(FILEINFO, "write message to session buff error");
		}
	}
}

void CSessionMgr::AddMessage(Safe_Smart_Ptr<Message> message)
{
	Safe_Smart_Ptr<Session> se;
	{
		GUARD_READ(CRWLock, obj, &m_seLock);
		map<int, Safe_Smart_Ptr<Session> >::iterator itTime = m_session.find(message->GetChannelID());
		if(itTime != m_session.end())
		{
			se = itTime->second;
		}
		else
		{
			return;
		}
	}

	packetHeader head;

	if(message->GetRemoteID() < 0)
	{
		message->SetRemoteID(se->GetRemoteID());
		message->SetRemoteType(se->GetRemoteType());
	}

	if(message->GetSecurity() != se->GetSecurity())
	{
		message->SetSecurity(se->GetSecurity());
	}

	message->EncryptMessage();
	message->GetHead(&head);

	if(se->Write(message->GetContent(), &head) == eCtlEpollErr)
	{
		LOG_BASE(FILEINFO, "write message to session buff error");
	}
}

}
