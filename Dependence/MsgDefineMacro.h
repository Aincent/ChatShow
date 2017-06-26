/*
 * MsgDefineMacro.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef MSGDEFINEMACRO_H_
#define MSGDEFINEMACRO_H_

#define NORMAL_MSG_SEND_SIZE (1024*32)

#define DEF_MSG_SIMPLE_DECLARE_FUN_H(x) void On_MSG_SIMPLE_##x(Safe_Smart_Ptr<CommBaseOut::Message> &message)
#define DEF_MSG_SIMPLE_DEFINE_FUN(t,x) void t::On_MSG_SIMPLE_##x(Safe_Smart_Ptr<CommBaseOut::Message> &message)
#define DEF_MSG_SIMPLE_REG_FUN(t,x) Message_Facade::RegisterSimpleMessage(t,x,this)

#define DEF_MSG_SIMPLE_DISPATCH_FUN(x) case x:\
	On_MSG_SIMPLE_##x(message); \
	break

#define DEF_MSG_REQUEST_DISPATCH_FUN(x) case x:\
	On_MSG_REQUEST_##x(message);\
	break

#define DEF_MSG_REQUEST_DECLARE_FUN_H(x) void On_MSG_REQUEST_##x(Safe_Smart_Ptr<CommBaseOut::Message> &message)
#define DEF_MSG_REQUEST_DEFINE_FUN(t,x) void t::On_MSG_REQUEST_##x(Safe_Smart_Ptr<CommBaseOut::Message> &message)
#define DEF_MSG_REQUEST_REG_FUN(t,x) Message_Facade::RegisterRequestHandler(t,x,this)

#define DEF_MSG_ACK_DISPATCH_FUN(x) case x:\
	On_MSG_ACK_##x(message, message->GetRequest(), message->GetRequest()->GetAct());\
	break

#define DEF_MSG_ACK_DECLARE_FUN_H(x) void On_MSG_ACK_##x(Safe_Smart_Ptr<CommBaseOut::Message> &message, Safe_Smart_Ptr<CommBaseOut::Message> &request, Safe_Smart_Ptr<NullBase> &act)
#define DEF_MSG_ACK_DEFINE_FUN(t,x) void t::On_MSG_ACK_##x(Safe_Smart_Ptr<CommBaseOut::Message> &message, Safe_Smart_Ptr<CommBaseOut::Message> &request, Safe_Smart_Ptr<NullBase> &act)
#define DEF_MSG_ACK_REG_FUN(t,x) Message_Facade::RegisterAckHandler(t,x,this)

#define DEF_SWITCH_TRY_DISPATCH_BEGIN try{\
CommBaseOut::int64 beginTime = 0; CommBaseOut::int64 endTime = 0; beginTime = CUtil::GetNowSecond();\
switch(message->GetMessageID())\
{

#define DEF_SWITCH_TRY_DISPATCH_END default :\
	{\
	LOG_ERROR(FILEINFO, "No Matching MessageID When Dispatching Message [messageID=%d remoteID=%d remoteType=%d]",message->GetMessageID(), message->GetRemoteID(), message->GetRemoteType());\
	break;\
	}\
	}\
	endTime = CUtil::GetNowSecond();\
	if(endTime - beginTime > 100)\
	{\
		LOG_FATAL(FILEINFO, "Message:[messageid = %d ] DEF_MSG_DISPATCH_FUN [ %s ] and too more time[%lld]", message->GetMessageID(),__FUNCTION__, endTime - beginTime);\
	}\
	}\
	catch(exception &e)\
	{\
	system("ps -aux | grep GameServer_d >> Memory.log");\
	LOG_ERROR(FILEINFO, "Message Deal With Error=[%s] When Dispatching Message [messageID=%d remoteID=%d remoteType=%d]",e.what() ,message->GetMessageID(), message->GetRemoteID(), message->GetRemoteType());\
	}\
	catch(...)\
	{\
	LOG_ERROR(FILEINFO, "Message Deal With Unknown Error When Dispatching Message [messageID=%d remoteID=%d remoteType=%d]",message->GetMessageID(), message->GetRemoteID(), message->GetRemoteType());\
	}\

#endif /* MSGDEFINEMACRO_H_ */
