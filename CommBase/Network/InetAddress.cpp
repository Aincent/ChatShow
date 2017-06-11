/*
 * InetAddress.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "InetAddress.h"

namespace CommBaseOut
{

Inet_Addr::Inet_Addr()
{
	memset(&m_in, 0, sizeof(m_in));
}

Inet_Addr::Inet_Addr(const string &ip, WORD port)
{
	memset(&m_in, 0, sizeof(m_in));

	m_in.sin_family = AF_INET;
	m_in.sin_port = htons(port);
	m_in.sin_addr.s_addr = inet_addr(ip.c_str());
}

Inet_Addr::Inet_Addr(const char *ip, WORD port)
{
	memset(&m_in, 0, sizeof(m_in));

	m_in.sin_family = AF_INET;
	m_in.sin_port = htons(port);
	m_in.sin_addr.s_addr = inet_addr(ip);
}

Inet_Addr::Inet_Addr(Inet_Addr *addr)
{
	memset(&m_in, 0, sizeof(m_in));

	m_in.sin_family = AF_INET;
	m_in.sin_port = htons(addr->GetPort());
	m_in.sin_addr.s_addr = inet_addr(addr->GetIPToChar());
}

Inet_Addr::Inet_Addr(struct sockaddr_in *addr)
{
	memset(&m_in, 0, sizeof(m_in));

	m_in.sin_family = AF_INET;
	m_in.sin_port = addr->sin_port;
	m_in.sin_addr.s_addr = addr->sin_addr.s_addr;
}

void Inet_Addr::SetIP(const string &ip)
{
	m_in.sin_addr.s_addr = inet_addr(ip.c_str());
}
void Inet_Addr::SetAnyIP()
{
	m_in.sin_addr.s_addr = htonl(INADDR_ANY);
}
void Inet_Addr::SetIP(const char *ip)
{
	m_in.sin_addr.s_addr = inet_addr(ip);
}
void Inet_Addr::SetIP(DWORD ip)
{
	m_in.sin_addr.s_addr = ip;
}
void Inet_Addr::SetPort(WORD port)
{
	m_in.sin_port = htons(port);
}
void Inet_Addr::SetInterPort(WORD port)
{
	m_in.sin_port = port;
}
void Inet_Addr::Set(Inet_Addr *addr)
{
	m_in.sin_port = addr->GetInterPort();
	m_in.sin_addr.s_addr = addr->GetIP();
}
void Inet_Addr::Set(struct sockaddr_in *addr)
{
	m_in.sin_port = addr->sin_port;
	m_in.sin_addr.s_addr = addr->sin_addr.s_addr;
}

string Inet_Addr::GetIPToString()
{
	string ip;

	ip = inet_ntoa(m_in.sin_addr);
	return ip;
}

const char * Inet_Addr::GetIPToChar()
{
	if(this == 0)
	{
		return 0;
	}

	return inet_ntoa(m_in.sin_addr);
}

DWORD Inet_Addr::GetIP()
{
	return m_in.sin_addr.s_addr;
}

WORD Inet_Addr::GetPort()
{
	return ntohs(m_in.sin_port);
}

WORD Inet_Addr::GetInterPort()
{
	return m_in.sin_port;
}

sockaddr_in * Inet_Addr::GetSocketAddr()
{
	return &m_in;
}

Inet_Addr * Inet_Addr::operator = (Inet_Addr &addr)
{
//	memset(&m_in, 0, sizeof(m_in));

	m_in.sin_family = AF_INET;
	m_in.sin_port = addr.GetInterPort();
	m_in.sin_addr.s_addr = addr.GetIP();

	return this;
}

Inet_Addr * Inet_Addr::operator = (const struct sockaddr_in &addr)
{
//	memset(&m_in, 0, sizeof(m_in));

	m_in.sin_family = AF_INET;
	m_in.sin_port = addr.sin_port;
	m_in.sin_addr.s_addr = addr.sin_addr.s_addr;

	return this;
}
}
