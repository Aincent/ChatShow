/*
 * Service_Handler.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef NETWORK_SERVICE_HANDLER_H_
#define NETWORK_SERVICE_HANDLER_H_

#include <map>
#include <vector>
#include "../define.h"
#include "../Smart_Ptr.h"
#include "../Common.h"
#include "../Thread/Task.h"
#include "InetAddress.h"
#include "Context.h"
#include "../Ref_Object.h"
#include "../Memory/MemAllocator.h"

using namespace std;

namespace CommBaseOut
{

class Task;
class Message;
class Message_Handler;
class Request_Handler;
class Ack_Handler;

class Message_Service_Handler
{
public:

	virtual ~Message_Service_Handler(){}

	virtual void on_new_channel_build(int channel_id,short int local_id,unsigned char local_type,short int remote_id,unsigned char remote_type,Safe_Smart_Ptr<Inet_Addr> remote_address) = 0;

	virtual void on_channel_error(int channel_id,short int local_id,unsigned char local_type,short int remote_id,unsigned char remote_type,int error_code,Safe_Smart_Ptr<Inet_Addr> remote_address) = 0;

	virtual void on_connect_failed(int connector_id,short int local_id,unsigned char local_type,short int remote_id,unsigned char remote_type,int error,Safe_Smart_Ptr<Inet_Addr> remote_address) = 0;

};

class HandlerManager
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:
	HandlerManager(Context *c);
	~HandlerManager();

	int RegisterMessageHandler(BYTE type, DWORD id, Message_Handler* handler);
	int RegisterRequestHandler(BYTE type, DWORD id, Request_Handler* handler);
	int RegisterAckHandler(BYTE type, DWORD id, Ack_Handler* handler);
	Message_Handler* GetMessageHandler(BYTE type, DWORD id);
	Request_Handler* GetRequestHandler(BYTE type, DWORD id);
	Ack_Handler* GetAckHandler(BYTE type, DWORD id);

private:
//	map<DWORD, Message_Handler* > m_messageHandler;
//	map<DWORD, Request_Handler* > m_requestHandler;
//	map<DWORD, Ack_Handler* > m_ackHandler;

	vector<Message_Handler* > m_messageHandler;
	vector<Request_Handler* > m_requestHandler;
	vector<Ack_Handler* > m_ackHandler;

	Context *m_c;
};
}

#endif /* NETWORK_SERVICE_HANDLER_H_ */
