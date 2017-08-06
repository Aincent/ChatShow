/*
 * FriendMessage.h
 *
 *  Created on: Jul 23, 2017
 *      Author: root
 */

#ifndef FRIENDMESSAGE_H_
#define FRIENDMESSAGE_H_

#include "Ref_Object.h"
#include "Smart_Ptr.h"
#include "SvrConfig.h"
#include "define.h"
#include "Common.h"
#include "Network/MessageHandler.h"
#include "MsgDefineMacro.h"
#include "ServerMsgDefine.h"
#include "../ServerManager.h"
#include "Timer/TimerInterface.h"
#include "../EventSet/EventDefine.h"

class FriendMessage : public Message_Handler, public Request_Handler, public Ack_Handler
{
public:
	~FriendMessage();

	static FriendMessage * GetInstance()
	{
		if(m_instance == 0)
		{
			m_instance = new FriendMessage();
		}

		return m_instance;
	}

	void DestroyInstance()
	{
		if(m_instance)
		{
			delete m_instance;
			m_instance = 0;
		}
	}

	virtual void Handle_Message(Safe_Smart_Ptr<Message> &message);
	virtual void Handle_Request(Safe_Smart_Ptr<Message> &message);
	virtual void Handle_Ack(Safe_Smart_Ptr<Message> &message);

public:
	//自己关注了某个人
	void watch(int64 selfID, int64 targetID);

	//取消清除关注列表
	void cancelWatch(int64 selfID, int64 targetID);
	void clearWatch(int64 charID);

	void dispatchEvent(int64 charID, int attrType, int value);

	int GetFriendWatchSelf(int64 charID, set<int64>& watchSelf);
private:
	FriendMessage();

	//param1: 触发事件的角色；  param2:响应事件的角色；  param3:属性类型；  param4:属性值
	void onEvent(int64 charID, int64 targetID, int attrType, int value);

	//下线
	bool offline(const EventArgs& e);

private:
	static FriendMessage * m_instance;

	std::map<int64,std::map<int64,int64> > m_applyList;	//申请添加好友列表
	std::map<int64,std::vector<int64> >		m_beApplyList;//被申请列表

	std::map<int64,std::set<int64> >				m_event_list;	//好友关注全局事件列表   某个玩家被谁关注了
	std::map<int64,std::set<int64> >				m_watch_list; //我关注的好友

	TimerConnection 	m_ClearTimer;													//清除申请列表定时器
	EventConnection 	m_OfflineEvent;												//下线事件
};

#endif /* FRIENDMESSAGE_H_ */
