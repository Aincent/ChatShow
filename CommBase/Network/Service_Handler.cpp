/*
 * Service_Handler.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "Service_Handler.h"
#include "../util.h"
#include "../Common.h"
#include "MessageManager.h"
#include "../Log/Logger.h"

namespace CommBaseOut
{

HandlerManager::HandlerManager(Context *c):m_c(c)
{
	m_messageHandler.resize(32 * 1024);
	m_requestHandler.resize(32 * 1024);
	m_ackHandler.resize(32 * 1024);
}

HandlerManager::~HandlerManager()
{
	m_messageHandler.clear();
	m_requestHandler.clear();
	m_ackHandler.clear();
}

int HandlerManager::RegisterMessageHandler(BYTE type, DWORD id, Message_Handler* handler)
{
	DWORD key = 0;

	key = (type << 16) | id;

	if(id >= 32 * 1024)
		return -1;

//	if(id > m_messageHandler.size())
//		m_messageHandler.resize(id + 1);

	m_messageHandler[id] = handler;

	return eNetSuccess;
}

int HandlerManager::RegisterRequestHandler(BYTE type, DWORD id, Request_Handler* handler)
{
	DWORD key = 0;

	key = (type << 16) | id;

	if(id >= 32 * 1024)
		return -1;

//	if(id > m_requestHandler.size())
//		m_requestHandler.resize(id + 1);

	m_requestHandler[id] = handler;

	return eNetSuccess;
}

int HandlerManager::RegisterAckHandler(BYTE type, DWORD id, Ack_Handler* handler)
{
	DWORD key = 0;

	key = (type << 16) | id;

	if(id >= 32 * 1024)
		return -1;

//	if(id > m_ackHandler.size())
//		m_ackHandler.resize(id + 1);

	m_ackHandler[id] = handler;

	return eNetSuccess;
}

Message_Handler* HandlerManager::GetMessageHandler(BYTE type, DWORD id)
{
//	DWORD key = 0;
//
//	key = (type << 16) | id;
//
//	map<DWORD, Message_Handler* >::iterator it = m_messageHandler.find(key);
//	if(it == m_messageHandler.end())
//	{
//		return 0;
//	}
//
//	return it->second;
	if(id >= m_messageHandler.size())
		return 0;

	return m_messageHandler[id];
}

Request_Handler* HandlerManager::GetRequestHandler(BYTE type, DWORD id)
{
//	DWORD key = 0;
//
//	key = (type << 16) | id;
//
//	map<DWORD, Request_Handler* >::iterator it = m_requestHandler.find(key);
//	if(it == m_requestHandler.end())
//	{
//		return 0;
//	}
//
//	return it->second;
	if(id >= m_requestHandler.size())
		return 0;

	return m_requestHandler[id];
}

Ack_Handler* HandlerManager::GetAckHandler(BYTE type, DWORD id)
{
//	DWORD key = 0;
//
//	key = (type << 16) | id;
//
//	map<DWORD, Ack_Handler* >::iterator it = m_ackHandler.find(key);
//	if(it == m_ackHandler.end())
//	{
//		return 0;
//	}
//
//	return it->second;
	if(id >= m_ackHandler.size())
		return 0;

	return m_ackHandler[id];
}
}
