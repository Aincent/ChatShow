/*
 * DispatchMessage.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef NETWORK_DISPATCHMESSAGE_H_
#define NETWORK_DISPATCHMESSAGE_H_

#include <deque>
#include <map>
#include <vector>
#include "../Thread/Task.h"
#include "../Smart_Ptr.h"
#include "../Common.h"
#include "../Singleton.h"
#include "../define.h"
#include "../Memory/MemAllocator.h"

using namespace std;

namespace CommBaseOut
{

class Message;
class Context;

class DispatchBase : public Task
#ifdef USE_MEMORY_POOL
, public MemoryBase
#endif
{
	typedef struct sAckItem : public Ref_Object
#ifdef USE_MEMORY_POOL
, public MemoryBase
#endif
	{
		int token;
		Safe_Smart_Ptr<Message> req;
		DWORD64 time;
	}AckItem;

public:
	DispatchBase(Context *c);
	virtual ~DispatchBase();

	virtual void MessageFlush(){}
	int AddAckTimeOut(Safe_Smart_Ptr<Message> &message);
	int DelAckTimeOut(Safe_Smart_Ptr<Message> &message);

	virtual void ProcessMessage(){}
	void AddMessage(Safe_Smart_Ptr<Message> & message);
	void AddMessage(deque<Safe_Smart_Ptr<Message> > & message);
	virtual void AddSendMessage(Safe_Smart_Ptr<Message> & message);
	void SetIndex(int index)
	{
		m_index = index;
	}

	int GetToken()
	{
//		GUARD(CSimLock, obj, &m_tockenLock);
		int tmp = m_token++;

		if(tmp < 0)
		{
			m_token = 1;
			tmp =  m_token++;
		}

		return tmp;
	}

	void Dispatch(Safe_Smart_Ptr<Message> & message);

protected:
	deque<Safe_Smart_Ptr<Message> > m_deque;
	CSimLock m_queLock;
	map<int, deque<Safe_Smart_Ptr<Message> > > m_sendDeque;
	CSimLock m_sendLock;
	Context *m_c;
	int m_index;

	map<int, Safe_Smart_Ptr<AckItem> > m_timeOut;
	CSimLock m_timeLock;
	int m_token;
//	CSimLock m_tockenLock;

	int64 m_timeNow;
};

class DispatchMessage : public DispatchBase
{
public:
	DispatchMessage(Context *c);
	~DispatchMessage();

//	virtual void AddSendMessage(Safe_Smart_Ptr<Message> & message);

	virtual void Close()
	{
		End();
	}

private:
	virtual int svr();
};


class SingleDispatchThread : public DispatchBase
{
public:
	SingleDispatchThread(Context *c);
	~SingleDispatchThread();

	void MessageFlush();

	virtual void ProcessMessage();

	virtual int Start(int num, pthread_attr_t *attr = 0)
	{
		return 0;
	}

	void Close()
	{
//		m_flag = true;
//		m_event.SetEvent();
//		m_flag = false;
		End();
	}

private:
	virtual int svr();

private:

	Event m_event;
};

class DispatchMgr
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:
	DispatchMgr(Context * c):m_index(0),m_blockThread(1),m_c(c)
	{

	}
	~DispatchMgr()
	{
		DeleteAll();
	}

	int Init(int count = 1);
	DispatchBase *  GetDispatch();
	DispatchBase *  GetDispatch(int index)
	{
		return m_allDispatch[index % m_allDispatch.size()];
	}
	void DeleteAll();

	int GetBlockThread()
	{
		return m_blockThread;
	}

private:

	vector<DispatchBase * > m_allDispatch;
	int m_index;
	int m_blockThread;
	Context *m_c;
};
}

#endif /* NETWORK_DISPATCHMESSAGE_H_ */
