/*
 * MessageBuild.cpp
 *
 *  Created on: Jun 29, 2017
 *      Author: root
 */

#include "MessageBuild.h"
#include "define.h"

Safe_Smart_Ptr<CommBaseOut::Message> build_message(int messageID, Safe_Smart_Ptr<CommBaseOut::Message> &message, google::protobuf::Message *content, int messageType, int timeOut)
{
	string tempStr;
	CommBaseOut::Message *newMessage = NEW CommBaseOut::Message(message);
	content->SerializePartialToString(&tempStr);

	newMessage->SetMessageType(messageType);
	newMessage->SetTimeout(timeOut);
	newMessage->SetMessageID(messageID);
	newMessage->SetContent(tempStr.c_str(), tempStr.size());

	return newMessage;
}

Safe_Smart_Ptr<CommBaseOut::Message> build_message(int messageID, google::protobuf::Message *content, SvrItem *handler, int messageType, int timeOut)
{
	string tempStr;
	CommBaseOut::Message *newMessage = NEW CommBaseOut::Message(handler->remoteID, handler->remoteType, handler->channelID);
	content->SerializePartialToString(&tempStr);

	newMessage->SetMessageType(messageType);
	newMessage->SetTimeout(timeOut);
	newMessage->SetMessageID(messageID);
	newMessage->SetLocalType(ServerConHandler::GetInstance()->GetLocalType());
	newMessage->SetContent(tempStr.c_str(), tempStr.size());

	return newMessage;
}
