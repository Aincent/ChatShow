/*
 * ConnManager.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef NETWORK_CONNMANAGER_H_
#define NETWORK_CONNMANAGER_H_

#include <map>
#include "../define.h"
#include "../Thread/Task.h"
#include "../Common.h"
#include "../Memory/MemAllocator.h"

using namespace std;

namespace CommBaseOut
{

enum EConnStatus
{
	eDisconnection,
	eConnetion,
	eReConnection,
	eDelConnection,
};

typedef struct
{
	int m_sock;
	BYTE m_status;
	BYTE m_index;
}ConnData;

class Context;

class CConnMgr : public Task
#ifdef USE_MEMORY_POOL
, public MemoryBase
#endif
{
public:
	CConnMgr(Context * c);
	~CConnMgr();

	int Init();

	int AddConn(int sock, int index);

	virtual void Close()
	{
//		m_flag = true;
//		m_event.SetEvent();
//		m_flag = false;
		End();
	}

private:
	virtual int svr();

private:
	map<int, ConnData> m_conn;
	CSimLock m_connLock;
	Context *m_c;
//	Event m_event;
};
}

#endif /* NETWORK_CONNMANAGER_H_ */
