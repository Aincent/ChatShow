/*
 * MessageBuild.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef MESSAGEBUILD_H_
#define MESSAGEBUILD_H_

#include "Smart_Ptr.h"
#include "Network/MessageManager.h"
#include "Network/NetWorkConfig.h"
#include "./google/protobuf/message.h"
#include "ServerManager.h"

using namespace std;
using namespace CommBaseOut;

Safe_Smart_Ptr<CommBaseOut::Message> build_message(int messageID, Safe_Smart_Ptr<CommBaseOut::Message> &message, google::protobuf::Message *content, int messageType = Ack, int timeOut = 10);

Safe_Smart_Ptr<CommBaseOut::Message> build_message(int messageID, google::protobuf::Message *content, SvrItem *handler, int messageType = Ack, int timeOut = 10);

Safe_Smart_Ptr<CommBaseOut::Message> build_message(int messageID, google::protobuf::Message *content, ConItem *handler, int messageType = Ack, int timeOut = 10);

Safe_Smart_Ptr<CommBaseOut::Message> build_message(int messageID, google::protobuf::Message *content, sClientItem *handler, int messageType = Ack, int timeOut = 10);


Safe_Smart_Ptr<CommBaseOut::Message> build_message(int messageID, char *content, int len, SvrItem *handler, int messageType = Ack, int timeOut = 10);


Safe_Smart_Ptr<CommBaseOut::Message> build_message(int messageID, string &content, sClientItem *handler, int messageType = Ack, int timeOut = 10);


Safe_Smart_Ptr<CommBaseOut::Message> build_message(int messageID, google::protobuf::Message *content, int channel, int messageType = Ack, int timeOut = 10);

Safe_Smart_Ptr<CommBaseOut::Message> build_message(int messageID, string &content, int channel, int messageType = Ack, int timeOut = 10);

Safe_Smart_Ptr<CommBaseOut::Message> build_message(int messageID, Safe_Smart_Ptr<CommBaseOut::Message> &message, int channel, int messageType = Ack, int timeOut = 10);

#endif /* MESSAGEBUILD_H_ */
