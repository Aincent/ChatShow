/*
 * MessageBuild.h
 *
 *  Created on: Jun 29, 2017
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

Safe_Smart_Ptr<CommBaseOut::Message> build_message(int messageID, Safe_Smart_Ptr<CommBaseOut::Message> &message, google::protobuf::Message *content, int messageType = Ack, int timeOut = 5);

Safe_Smart_Ptr<CommBaseOut::Message> build_message(int messageID, google::protobuf::Message *content, SvrItem *handler, int messageType = Ack, int timeOut = 5);


#endif /* MESSAGEBUILD_H_ */
