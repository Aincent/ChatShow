/*
 * ConnManager.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#include "ConnManager.h"
#include "NetWorkConfig.h"
#include "Context.h"
#include "../util.h"
#include "EpollMain.h"
#include "Session.h"
#include "../Smart_Ptr.h"
#include "../Log/Logger.h"
#include "Service_Handler.h"
#include "MessageManager.h"

namespace CommBaseOut
{

CConnMgr::CConnMgr(Context * c):m_c(c)
{

}

CConnMgr::~CConnMgr()
{
//	if(!m_flag)
//	{
//		m_flag = true;
//		m_event.SetEvent();
//		m_flag = false;
		End();
//	}

	m_conn.clear();
}

int CConnMgr::Init()
{
	int index = 0;
	vector<ConnectionConfig> vec = m_c->GetAllConnConfig();

	vector<ConnectionConfig>::iterator it = vec.begin();
	for(; it!=vec.end(); ++it)
	{
		ConnData con;
		int count = (*it).group_count;
		if(count == 0)
			count = 1;

		for(int i=0; i<count; ++i)
		{
			int sock = socket(AF_INET, SOCK_STREAM, 0);
			if(sock == -1)
			{
				LOG_BASE(FILEINFO, "create socket error");

				return eCreateSockErr;
			}

			linger sockLinger;
			sockLinger.l_onoff = 1;
			sockLinger.l_linger = 0;
			setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char*)&sockLinger, sizeof(linger));

			int nOpt = 1;
			setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void*)&nOpt, sizeof(nOpt));
			setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const void*)&nOpt, sizeof(nOpt));

			con.m_sock = sock;
			con.m_status = eDisconnection;
			con.m_index = index;

			m_conn[con.m_sock] = con;
		}

		++index;
	}

	return eNetSuccess;
}

int CConnMgr::AddConn(int sock, int index)
{
	ConnData con;
	con.m_sock = sock;
	con.m_status = eReConnection;
	con.m_index = index;
	{
		GUARD(CSimLock, obj, &m_connLock);
		m_conn[sock] = con;
	}

//	m_event.SetEvent();

	return eNetSuccess;
}

int CConnMgr::svr()
{
	while(!m_flag)
	{
		{
			GUARD(CSimLock, obj, &m_connLock);
			if(m_conn.size() == 0)
			{
				obj.UnLock();
//				m_event.WaitForSingleEvent();
				sleep(1);
				continue;
			}

			map<int, ConnData>::iterator it = m_conn.begin();
			for(; it!=m_conn.end();)
			{
				ConnectionConfig * conf = m_c->GetConnConfig(it->second.m_index);
				if(it->second.m_status == eDisconnection || (it->second.m_status == eReConnection && conf->re_connect_))
				{
					struct sockaddr_in addrIn;

					addrIn.sin_family = AF_INET;
					addrIn.sin_port = conf->remote_address_->GetInterPort();

					if(conf->remote_address_->GetIPToString() == "")
					{
						string netName = "lo";

						addrIn.sin_addr.s_addr = inet_addr(CUtil::GetIpByName(it->first, netName).c_str());
					}
					else
					{
						addrIn.sin_addr.s_addr = conf->remote_address_->GetIP();
					}

					if(connect(it->first, (sockaddr *)&addrIn, sizeof(struct sockaddr)) < 0)
					{
						it->second.m_status = eReConnection;
						++it;
						LOG_BASE(FILEINFO, " connect Server socket but connect error rtype[%d] ip[%s] port[%d]", conf->remote_type_, conf->remote_address_->GetIPToChar(), conf->remote_address_->GetPort());
//						m_c->SystemErr(it->first, conf->local_id_, conf->local_type_, conf->remote_id_, conf->remote_type_, eConnErr, conf->remote_address_->GetIPToString(),conf->remote_address_->GetPort());

						continue;
						//return eConnErr;
					}

					CEpollLoop * loop = m_c->GetEpollMgr()->GetEpollLoop(it->first);
					if(loop == 0)
					{
						continue;
					}
					else
					{
						request_package rPackage;

//						printf("\n++++++++++++++ fd = %d index = %d+++++++++++\n", it->first,it->second.m_index);
						rPackage.u.conn.fd = it->first;
						rPackage.u.conn.index = it->second.m_index;

						rPackage.header[6] = eRequestConn;
						rPackage.header[7] = sizeof(rPackage.u.conn);

						loop->SendConnRequest(rPackage, sizeof(rPackage.u.conn) + 2);
						CUtil::MSleep(50);
					}
				}

				m_conn.erase(it++);
			}
		}

		sleep(1);
	}

	return eNetSuccess;
}
}
