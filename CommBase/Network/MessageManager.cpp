/*
 * MessageManager.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "MessageManager.h"
#include "Context.h"
#include "../util.h"
#include "Session.h"
#include "Service_Handler.h"
#include "../Log/Logger.h"
#include <assert.h>
#include "EpollMain.h"
#include "../Encrypt/md5.h"

namespace CommBaseOut
{

Message::Message():m_remoteID(-1),m_remoteType(0),
		m_localID(0),m_localType(0),m_timeout(0),m_reqID(0),m_channelID(-1),
		m_messageID(0),m_messageType(0),m_length(0),m_errno(0),m_security(false),m_sendTime(0),m_loopIndex(-1),m_group(-1),m_groupKey(-1)
{
//	m_content = NEW_BASE(char, MAX_RECV_MSG_CONTENT_SIZE);// char[MAX_RECV_MSG_SIZE];
//	m_content = (char*)NEW_BASE(char, MAX_RECV_MSG_CONTENT_SIZE);// char[MAX_RECV_MSG_SIZE];
	m_content = (char *)malloc(MAX_RECV_MSG_CONTENT_SIZE * sizeof(char));

	m_addr = NEW Inet_Addr;
}
Message::Message(int id, unsigned int type):m_remoteID(id),m_remoteType(type),
		m_localID(0),m_localType(0),m_timeout(0),m_reqID(0),m_channelID(-1),
		m_messageID(0),m_messageType(0),m_length(0),m_errno(0),m_security(false),m_sendTime(0),m_loopIndex(-1),m_group(-1),m_groupKey(-1)
{
//	m_content = NEW_BASE(char, MAX_RECV_MSG_CONTENT_SIZE);
	m_content = (char *)malloc(MAX_RECV_MSG_CONTENT_SIZE * sizeof(char));

	m_addr = NEW Inet_Addr;
}

Message::Message(int id, unsigned int type, int channel):m_remoteID(id),m_remoteType(type),
		m_localID(0),m_localType(0),m_timeout(0),m_reqID(0),m_channelID(channel),
		m_messageID(0),m_messageType(0),m_length(0),m_errno(0),m_security(false),m_sendTime(0),m_loopIndex(-1),m_group(-1),m_groupKey(-1)
{
//	m_content = NEW_BASE(char, MAX_RECV_MSG_CONTENT_SIZE);
	m_content = (char *)malloc(MAX_RECV_MSG_CONTENT_SIZE * sizeof(char));

	m_addr = NEW Inet_Addr;
}

Message::Message(int channel):m_remoteID(-1),m_remoteType(0),
		m_localID(0),m_localType(0),m_timeout(0),m_reqID(0),m_channelID(channel),
		m_messageID(0),m_messageType(0),m_length(0),m_errno(0),m_security(false),m_sendTime(0),m_loopIndex(-1),m_group(-1),m_groupKey(-1)
{
//	m_content = NEW_BASE(char, MAX_RECV_MSG_CONTENT_SIZE);
	m_content = (char *)malloc(MAX_RECV_MSG_CONTENT_SIZE * sizeof(char));

	m_addr = NEW Inet_Addr;
}

Message::Message(int id, unsigned int type, int group, int64 key):m_remoteID(id),m_remoteType(type),
		m_localID(0),m_localType(0),m_timeout(0),m_reqID(0),m_channelID(-1),
		m_messageID(0),m_messageType(0),m_length(0),m_errno(0),m_security(false),m_sendTime(0),m_loopIndex(-1),m_group(group),m_groupKey(key)
{
//	m_content = NEW_BASE(char, MAX_RECV_MSG_CONTENT_SIZE);
	m_content = (char *)malloc(MAX_RECV_MSG_CONTENT_SIZE * sizeof(char));

	m_addr = NEW Inet_Addr;
}

Message::Message(Safe_Smart_Ptr<Message> &message):m_remoteID(message->GetRemoteID()),m_remoteType(message->GetRemoteType()),
		m_localID(message->GetLocalID()),m_localType(message->GetLocalType()),m_timeout(0),m_reqID(message->GetReqID()),m_channelID(message->GetChannelID()),
		m_messageID(0),m_messageType(0),m_length(0),m_errno(0),m_security(false),m_sendTime(0),m_loopIndex(message->GetLoopIndex()),m_group(message->GetGroup()),m_groupKey(message->GetGroupKey())
{
//	m_content = NEW_BASE(char, MAX_RECV_MSG_CONTENT_SIZE);
	m_content = (char *)malloc(MAX_RECV_MSG_CONTENT_SIZE * sizeof(char));

	m_addr = NEW Inet_Addr;
}

Message::~Message()
{
	if(m_content)
	{
//		DELETE_BASE(m_content, eMemoryArray);
		free(m_content);
		m_content = 0;
	}
}

void Message::SetHead(packetHeader &head)
{
	m_length = head.length;
	m_messageID = head.messageID;
	m_messageType = head.messageType;
	m_localID = head.remoteID;
	m_localType = head.remoteType;
	m_remoteType = head.localType;
	m_reqID = head.reqID;
	m_security = head.security;
	m_sendTime = head.sendTime;
}
void Message::GetHead(packetHeader * head)
{
	head->length = m_length;
	head->messageID = m_messageID;
	head->messageType = m_messageType;
	head->remoteID = m_remoteID;
	head->remoteType = m_remoteType;
	head->localType = m_localType;
	head->reqID = m_reqID;
	head->security = m_security;
	head->sendTime = m_sendTime;

	head->toSmallEndian();
}

void Message::SetContent(const char *content, int len)
{
	//16为md5串
	if((len + HEADER_LENGTH + 16) > MAX_MSG_PACKET_SIZE)
	{
		LOG_BASE(FILEINFO, "Message Set Content to Big[%d]", len);

		return ;
	}

	if((len + 16) > MAX_RECV_MSG_CONTENT_SIZE)
	{
		m_content = (char *)realloc(m_content, len + 16);
	}

	CUtil::SafeMemmove(m_content, len, content, len);

	m_length = len;
}
char *Message::GetContent()
{
	return m_content;
}

void Message::EncryptMessage()
{
	if(m_security)
	{
		if(m_length + 16 > MAX_MSG_PACKET_SIZE)
		{
			LOG_BASE(FILEINFO, "content encrypted beyond max len");

			return;
		}

		MD5 md5(m_content, m_length);

		CUtil::SafeMemmove(m_content + m_length, MAX_MSG_PACKET_SIZE - m_length, md5.Digest(), 16);
		m_length += 16;
	}
}

bool Message::UnEncryptMessage()
{
	if(m_security)
	{
		MD5 md5(m_content, m_length - 16);
		char ttt[32] = {0};

		CUtil::SafeMemmove(ttt,32,m_content + 3, 16);

		if(strncmp((const char *)md5.Digest(), m_content + m_length - 16, 16) != 0)
		{
			return false;
		}

		m_length -= 16;
	}

	return true;
}

}
