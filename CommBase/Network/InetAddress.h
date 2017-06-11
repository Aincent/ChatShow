/*
 * InetAddress.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef NETWORK_INETADDRESS_H_
#define NETWORK_INETADDRESS_H_

#include <sys/socket.h>
#include <sys/types.h>
#include "../define.h"
#include <string>
#include "../Ref_Object.h"
#include "../Memory/MemAllocator.h"

using namespace std;

namespace CommBaseOut
{

class Inet_Addr : public Ref_Object
#ifdef USE_MEMORY_POOL
, public MemoryBase
#endif
{
public:
	Inet_Addr();
	Inet_Addr(const string &ip, WORD port);
	Inet_Addr(const char *ip, WORD port);
	Inet_Addr(Inet_Addr *addr);
	Inet_Addr(struct sockaddr_in *addr);

	void SetIP(const string &ip);
	void SetAnyIP();
	void SetIP(const char *ip);
	void SetIP(DWORD ip);
	void SetPort(WORD port);
	void SetInterPort(WORD port);
	void Set(Inet_Addr *addr);
	void Set(struct sockaddr_in *addr);

	Inet_Addr * operator = (Inet_Addr &addr);
	Inet_Addr * operator = (const struct sockaddr_in &addr);

	string GetIPToString();
	const char * GetIPToChar();
	DWORD GetIP();
	WORD GetPort();
	WORD GetInterPort();
	sockaddr_in *GetSocketAddr();

private:
	struct sockaddr_in m_in;
};
}

#endif /* NETWORK_INETADDRESS_H_ */
