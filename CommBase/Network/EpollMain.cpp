/*
 * EpollMain.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "EpollMain.h"
#include "NetWorkConfig.h"
#include "Context.h"
#include "Service_Handler.h"
#include "../Log/Logger.h"
#include "DispatchMessage.h"
#include "MessageManager.h"

namespace CommBaseOut
{

CEpollLoop::CEpollLoop(Context *c):m_c(c),m_epollFd(-1),m_index(0),m_session(c),m_dispatch(0)
{
}
CEpollLoop::~CEpollLoop()
{
	close(m_epollFd);
	m_session.DeleteAll();
	close(m_accfd[0]);
	close(m_accfd[1]);
	close(m_confd[0]);
	close(m_confd[1]);
}


int CEpollLoop::SetMaxFile(int nMax)
{
//	struct rlimit rt;
//
//	rt.rlim_cur = nMax;
//	rt.rlim_max = rt.rlim_cur;
//	if(setrlimit(RLIMIT_NOFILE, &rt) < 0)
//	{
//		return eLimitFdErr;
//	}

	return eNetSuccess;
}

int CEpollLoop::Init(unsigned char index)
{
	if(SetMaxFile(MAX_EPOLL_fd) != eNetSuccess)
	{
		LOG_BASE(FILEINFO, "set max file error");

		return eLimitFdErr;
	}

	m_epollFd = epoll_create(MAX_EPOLL_EVENT);
	if(m_epollFd == -1)
	{
		LOG_BASE(FILEINFO, "create epoll error");

		return eCreateEpollErr;
	}

	m_index = index;

	if(pipe(m_accfd) || pipe(m_confd))
	{
		return eLimitFdErr;
	}

	return eNetSuccess;
}

void CEpollLoop::ExeCommand()
{
	struct timeval tv = {0,0};
	int retval;
	int fd = ((m_accfd[0] > m_confd[0]) ? m_accfd[0] : m_confd[0]);

	FD_ZERO(&m_rfds);
	FD_SET(m_accfd[0], &m_rfds);
	FD_SET(m_confd[0], &m_rfds);

	retval = select(fd+1, &m_rfds, NULL, NULL, &tv);
	if(retval > 0)
	{
		if(FD_ISSET(m_accfd[0], &m_rfds))
		{
			ReadCommd(m_accfd[0]);
		}

		if(FD_ISSET(m_confd[0], &m_rfds))
		{
			ReadCommd(m_confd[0]);
		}
	}
}

void CEpollLoop::BlockReadPipe(int pipefd, void *buffer, int sz)
{
	for (;;)
	{
		int n = read(pipefd, buffer, sz);
		if (n<0)
		{
			if (errno == EINTR)
				continue;

			LOG_BASE(FILEINFO, "socket-server : read pipe error %s.",strerror(errno));

			return;
		}

		return;
	}
}

void CEpollLoop::ReadCommd(int fd)
{
	BYTE buffer[256] = {0};
	BYTE header[2] = {0};
	int res = 0;

	BlockReadPipe(fd, header, sizeof(header));

	int type = header[0];
	int len = header[1];

	if(len <= 0)
		return;

	BlockReadPipe(fd, buffer, len);

	switch(type)
	{
	case eRequestClose:
		{
			break;
		}
	case eRequestListen:
		{
			struct request_listen * rlisten = (struct request_listen *)buffer;
			struct sockaddr_in addrIn;
			AcceptorConfig * accConf = m_c->GetAccConfig(rlisten->index);

			addrIn.sin_family = AF_INET;
			addrIn.sin_port = rlisten->opaque;
			addrIn.sin_addr.s_addr = inet_addr(rlisten->host);

			EpollEventPtr * se = m_session.AddTimerSessionEx(rlisten->fd, addrIn, accConf, (rlisten->index << 8) | (WORD)m_index, eAcceptEpoll);
			if(se == 0)
			{
//				m_c->SystemErr(rlisten->fd, accConf->local_id_, accConf->local_type_, 0, 0, eAddSeTimeoutErr, rlisten->host,rlisten->opaque);
				LOG_BASE(FILEINFO, "accept success but add timersession error ip[%s]", rlisten->host);
				close(rlisten->fd);

				return;
			}

			struct epoll_event ev;

			bzero(&ev, sizeof(ev));
			ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLERR | EPOLLHUP/* | EPOLLOUT*/;
			ev.data.ptr = (void *)se;
			if((res = EpollCtl(EPOLL_CTL_ADD, rlisten->fd, &ev)) != eNetSuccess)
			{
				LOG_BASE(FILEINFO, "eRequestListen epollctr error [%d]", rlisten->fd);
				m_session.DeleteSession(rlisten->fd, res);
				se->se = 0;
			}
			break;
		}
	case eRequestConn:
		{
			struct request_conn * rConn = (struct request_conn *)buffer;
			struct sockaddr_in addrIn;
			ConnectionConfig * connConf = m_c->GetConnConfig(rConn->index);

			addrIn.sin_family = AF_INET;
			addrIn.sin_port = connConf->remote_address_->GetPort();
			addrIn.sin_addr.s_addr = inet_addr(connConf->remote_address_->GetIPToString().c_str());

			EpollEventPtr * se = m_session.AddTimerSessionEx(rConn->fd, addrIn, connConf, (rConn->index << 8) | (WORD)m_index, eConnEpoll);
			if(se == 0)
			{
				LOG_BASE(FILEINFO, "connect success but add timersession error ip[%s] port[%d]", connConf->remote_address_->GetIPToChar(), connConf->remote_address_->GetPort());
//				m_c->SystemErr(rConn->fd, connConf->local_id_, connConf->local_type_, 0, 0, eAddSeTimeoutErr, connConf->remote_address_->GetIPToString(),connConf->remote_address_->GetPort());
				close(rConn->fd);

				return;
			}

			struct epoll_event ev;

//			printf("\n---------------create ptr=%x session=%x----------------------------", se, se->se.Get());
			bzero(&ev, sizeof(ev));
			ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLERR | EPOLLHUP/* | EPOLLOUT*/;
//				ev.data.fd = nSocketClient;
			ev.data.ptr = (void *)se;
			if((res = EpollCtl(EPOLL_CTL_ADD, rConn->fd, &ev)) != eNetSuccess)
			{
				LOG_BASE(FILEINFO, "eRequestConn epollctr error [%d]", rConn->fd);
				m_session.DeleteSession(rConn->fd, res);
				se->se = 0;
//					printf("\n=======================accept delete session = %x =========================\n", se->se);
				return;
			}

			Safe_Smart_Ptr<Message> message = NEW Message();
			char con[8] = {0};

			message->SetSecurity(connConf->security_);
			message->SetRemoteID(connConf->remote_id_);
			message->SetRemoteType(connConf->remote_type_);
			message->SetChannelID(rConn->fd);

			CUtil::SafeMemmove(con, 8, &connConf->local_type_, 1);
			CUtil::SafeMemmove(con+1, 8, &connConf->local_id_, 2);
			if(connConf->group_count <= 1)
			{
				message->SetMessageID(C2SMessage);
				message->SetContent(con, 3);
			}
			else
			{
				CUtil::SafeMemmove(con+3, 8, &connConf->group_count, 1);
				message->SetMessageID(C2SGroupMessage);
				message->SetContent(con, 4);
			}

			message->SetMessageType(ConnMessage);
//						message->SetMessageTime(CUtil::GetNowSecond());
			message->SetLocalType(connConf->local_type_);
			se->se->Write(message);

			break;
		}
	}
}

void CEpollLoop::SendAccRequest(struct request_package &request, int len)
{
	for (;;)
	{
		int n = write(m_accfd[1], &request.header[6], len);
		if (n<0)
		{
			if (errno != EINTR)
			{
				LOG_BASE(FILEINFO, "socket-server : send ctrl command error %s.\n", strerror(errno));
			}

			continue;
		}

		return;
	}
}

void CEpollLoop::SendConnRequest(struct request_package &request, int len)
{
	for (;;)
	{
		int n = write(m_confd[1], &request.header[6], len);
		if (n<0)
		{
			if (errno != EINTR)
			{
				LOG_BASE(FILEINFO, "socket-server : send ctrl command error %s.\n", strerror(errno));
			}

			continue;
		}

		return;
	}
}

int CEpollLoop::svr()
{
	socklen_t socklen;
	socklen = sizeof(struct sockaddr);

	bool bExe = false;
	bool bRecvMsg = true;
	int nHadTimeOut = 0;
	int nEventsNum = 0;
	struct epoll_event events[MAX_EPOLL_EVENT];

	while(!m_flag)
	{
		int64 bnow = 0;
		int64 enow = 0;
		int evcount = 0;

		{
			ExeCommand();
		}

		{
			//超时列表
			m_session.Tick();
		}

		nEventsNum = epoll_wait(m_epollFd, events, MAX_EPOLL_EVENT, EPOLL_TIMEOUT - 990);
		if (nEventsNum > 0)
		{
			bnow = CUtil::GetNowMicrosecod();
			bRecvMsg = true;
			for (int nIndex = 0; nIndex < nEventsNum; ++nIndex)
			{
				evcount++;
				bExe = false;
				if( (events[nIndex].events & EPOLLIN) && !(events[nIndex].events & EPOLLRDHUP))
				{
					bExe = true;
					sEpollEventPtr * se = (sEpollEventPtr *)events[nIndex].data.ptr;
					if(se != 0)
					{
						int nRet = se->se->Recv();
						if(nRet == eSocketClose)
						{
							m_c->GetEpollMgr()->DeleteChannelEx(se->se->GetSock(), eSocketClose);

							LOG_BASE(FILEINFO, "recv message but return is socket[%d] close", se->se->GetSock());
						}
					}
					else
					{
//						printf("\naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
					}
				}

				if(events[nIndex].events & EPOLLOUT)
				{
					bExe = true;
					bRecvMsg = false;
					sEpollEventPtr * se = (sEpollEventPtr *)events[nIndex].data.ptr;
					if(se != 0)
					{
						int nRet = se->se->Send();
						if(nRet == eSocketClose)
						{
							m_c->GetEpollMgr()->DeleteChannelEx(se->se->GetSock(), eSocketClose);

							LOG_BASE(FILEINFO, "Send message but return is socket[%d] close", se->se->GetSock());
						}
					}
					else
					{
//						printf("\nbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");
					}
				}

				if((events[nIndex].events & EPOLLRDHUP) || (events[nIndex].events & EPOLLERR) || (events[nIndex].events & EPOLLHUP))
				{
					sEpollEventPtr * se = (sEpollEventPtr *)events[nIndex].data.ptr;
					if(se != 0)
					{
						bExe = true;
						m_c->GetEpollMgr()->DeleteChannelEx(se->se->GetSock(), eEpollClose);
						LOG_BASE(FILEINFO, "client close channel[%d] event ", se->se->GetSock());
					}
					else
					{
//						printf("\ncccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\n");
					}
				}

				if (!bExe)
				{
					sEpollEventPtr * se = (sEpollEventPtr *)events[nIndex].data.ptr;
					if(se != 0)
					{
						m_c->GetEpollMgr()->DeleteChannelEx(se->se->GetSock(), eEpollNull);
						LOG_BASE(FILEINFO, "client channel[%d] null event ", se->se->GetSock());
					}
					else
					{
//						printf("\ndddddddddddddddddddddddddddddddddddddddddddddddddddddddd\n");
					}
				}
			}
			enow = CUtil::GetNowMicrosecod();
//			if(enow - bnow > 100)
//				LOG_BASE(FILEINFO, "epoll[index=%d, count = %d] main time = %lld", m_index, evcount, enow - bnow);
//				printf("\n\n &&&&&&&&&&&&&&&&&&&&&&& epoll main time = %lld &&&&&&&&&&&&&&&&&&&&&&&\n\n", enow - bnow);

			if(bRecvMsg)
			{
				++nHadTimeOut;
			}
			else
			{
				nHadTimeOut = 0;
			}
		}
		else
		{
			++nHadTimeOut;
			if (nEventsNum < 0 && errno != EINTR)
			{
				break;
			}
		}

//		usleep(10 * 1000);
	}// end while

	return eNetSuccess;
}

void CEpollLoop::DeleteSession(int fd, int err)
{
	struct epoll_event ev = {0,{0}};

	EpollCtl(EPOLL_CTL_DEL, fd, &ev);
	m_session.DeleteSession(fd, err);
}

void CEpollLoop::DeleteSessionEx(int fd, int err)
{
	struct epoll_event ev = {0,{0}};

	EpollCtl(EPOLL_CTL_DEL, fd, &ev);
	m_session.DeleteSessionEx(fd, err);
}

int CEpollLoop::EpollCtl(int nOp, int nSocket, struct epoll_event *event)
{
	int res = -1;

	if(EPOLL_CTL_ADD == nOp)
	{
		if ((res = SetNonBlocking(nSocket)) != eNetSuccess)
		{
			LOG_BASE(FILEINFO, "epoll set nonblock error");

			return res;
		}

		if((res =SetSockKeepAlive(nSocket, KEEP_IDLE, KEEP_INTERVAL, KEEP_COUNT)) != eNetSuccess)
		{
			LOG_BASE(FILEINFO, "epoll set keepalive error");

			return res;
		}
	}

	if(epoll_ctl(m_epollFd, nOp, nSocket, event) < 0)
	{
		LOG_BASE(FILEINFO, "epoll add socket event error");

		return eCtlEpollErr;
	}

	return eNetSuccess;
}

int CEpollLoop::SetNonBlocking(int nSock)
{
	int nOpts;

	if((nOpts = fcntl(nSock, F_GETFL)) < 0)
	{
		return eGetSocketErr;
	}

	nOpts |= O_NONBLOCK;

	if(fcntl(nSock, F_SETFL, nOpts) < 0)
	{
		return eSetSocketErr;
	}

	return eNetSuccess;
}

int CEpollLoop::SetSockKeepAlive(int nSock, int nKeepIdle, int nKeepInterval, int nKeepCount)
{
	int nKeepAlive = 1;
	if(setsockopt(nSock, SOL_SOCKET, SO_KEEPALIVE, (void *)&nKeepAlive, sizeof(nKeepAlive)) != 0)
	{
		return eSetSocketErr;
	}
	if(setsockopt(nSock, SOL_TCP, TCP_KEEPIDLE, (void *)&nKeepIdle, sizeof(nKeepIdle)) != 0)
	{
		return eSetSocketErr;
	}
	if(setsockopt(nSock, SOL_TCP, TCP_KEEPINTVL, (void *)&nKeepInterval, sizeof(nKeepInterval)) != 0)
	{
		return eSetSocketErr;
	}
	if(setsockopt(nSock, SOL_TCP, TCP_KEEPCNT, (void *)&nKeepCount, sizeof(nKeepCount)) != 0)
	{
		return eSetSocketErr;
	}

	return eNetSuccess;
}

CEpollMain::CEpollMain(Context *c):m_c(c)
{

}

CEpollMain::~CEpollMain()
{
	vector<CEpollLoop *>::iterator it = m_epollLoop.begin();
	for(; it!=m_epollLoop.end(); ++it)
	{
		(*it)->End();
		delete (*it);
		(*it) = 0;
	}

	m_epollLoop.clear();
}

int CEpollMain::AddEpollLoop(unsigned char index)
{
	CEpollLoop *loop = NEW CEpollLoop(m_c);
	int res = -1;

	if((!loop || (res=loop->Init(index)) != eNetSuccess))
	{
		LOG_BASE(FILEINFO, "init epoll error");

		return res;
	}

	if((res=loop->Start(1)) != eNetSuccess)
	{
		LOG_BASE(FILEINFO, "start epoll error");

		return res;
	}

	m_epollLoop.push_back(loop);

	return eNetSuccess;
}

int CEpollMain::DeleteAll()
{
	vector<CEpollLoop *>::iterator it = m_epollLoop.begin();
	for(; it!=m_epollLoop.end(); ++it)
	{
		(*it)->End();
		delete (*it);
		(*it) = 0;
	}

	m_epollLoop.clear();


	return eNetSuccess;
}

void CEpollMain::AddChannel(int fd, int index)
{
	GUARD_WRITE(CRWLock, obj, &m_channelLock);
	m_channelEpoll[fd] = index;
}

void CEpollMain::DeleteChannelEx(int fd, int err)
{
#ifdef DEBUG
	int64 beTime = CUtil::GetNowSecond();
#endif
	{
		CEpollLoop * tLoop = GetEpollLoop(fd);
		if(tLoop == 0)
		{
			return;
		}

		tLoop->DeleteSessionEx(fd, err);
	}

#ifdef DEBUG
	int64 endTime = CUtil::GetNowSecond();
	if(endTime - beTime > 200)
	{
		LOG_BASE(FILEINFO, "delete session time = %lld", endTime - beTime);
	}
#endif

}

void CEpollMain::DeleteChannel(int fd, int err)
{
#ifdef DEBUG
	int64 beTime = CUtil::GetNowSecond();
#endif
	{
		CEpollLoop * tLoop = GetEpollLoop(fd);
		if(tLoop == 0)
		{
			return;
		}

		tLoop->DeleteSession(fd, err);
	}

#ifdef DEBUG
	int64 endTime = CUtil::GetNowSecond();
	if(endTime - beTime > 200)
	{
		LOG_BASE(FILEINFO, "delete session time = %lld", endTime - beTime);
	}
#endif

}

WORD CEpollMain::GetEpollIndex(int fd)
{
	GUARD_READ(CRWLock, obj, &m_channelLock);
	map<int, WORD>::iterator it = m_channelEpoll.find(fd);
	if(m_channelEpoll.end() != it)
	{
		return it->second;
	}

	return 0;
}
}
