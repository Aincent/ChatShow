/*
 * MessageHandler.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef NETWORK_MESSAGEHANDLER_H_
#define NETWORK_MESSAGEHANDLER_H_

#include "../Common.h"
#include "MessageManager.h"

namespace CommBaseOut
{

class Message_Handler
{
public:
	virtual ~Message_Handler(){}

	virtual void Handle_Message(Safe_Smart_Ptr<Message> &message) = 0;

};

class Request_Handler
{
public:
	virtual ~Request_Handler(void){}

	virtual void Handle_Request(Safe_Smart_Ptr<Message> &message) = 0;
};

class Ack_Handler
{
public:
	virtual ~Ack_Handler(void){}

	virtual void Handle_Ack(Safe_Smart_Ptr<Message>& ack) = 0;
};

}

#endif /* NETWORK_MESSAGEHANDLER_H_ */
