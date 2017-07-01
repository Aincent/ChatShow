/*
 * Message_Facade.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef NETWORK_MESSAGE_FACADE_H_
#define NETWORK_MESSAGE_FACADE_H_

#include "NetWorkConfig.h"
#include "../Smart_Ptr.h"
#include "Service_Handler.h"
#include "../Common.h"

namespace CommBaseOut
{

class Context;
class Message_Service_Handler;
class Message;


/*
 * 逻辑层与网络层交互类
 * 封装所有的交互接口
 */
class Message_Facade
{
public:

	/*
	 * 初始化
	 * return :E_NetWorkRet
	 * param : mh 连接处理     blockThread 消息分发处理线程数(值为1表示同步处理消息)     ioThread 消息接收和发送的线程数
	 */
	static int Init(Message_Service_Handler *mh, int blockThread = 1, int ioThread = 1);

	/*
	 * 添加一条需要监听的端口设置
	 *return :
	 *param : 设置
	 */
	static void AddAcceptConfig(AcceptorConfig &conf);

	/*
	 * 添加一条需要连接设置
	 *return :
	 *param : 设置
	 */
	static void AddConnectionConfig(ConnectionConfig &conf);

	/*
	 * 开启网络
	 * return :E_NetWorkRet
	 * param :
	 */
	static int Run();

	/*
	 * 注册简单消息
	 * return :
	 * param : type接受消息的服务器类型    id 消息id（最多只能占三个字节）    handler接受消息对象
	 */
	static void RegisterSimpleMessage(unsigned char type, unsigned int id, Message_Handler * handler)
	{
		m_c->GetHandlerMgr()->RegisterMessageHandler(type, id, handler);
	}

	/*
	 * 注册请求消息
	 * return :
	 * param : type接受消息的服务器类型    id 消息id（最多只能占三个字节）    handler接受消息对象
	 */
	static void RegisterRequestHandler(unsigned char type, unsigned int id, Request_Handler * handler)
	{
		m_c->GetHandlerMgr()->RegisterRequestHandler(type, id, handler);
	}

	/*
	 * 注册回复消息
	 * return :
	 * param : type接受消息的服务器类型    id 消息id（最多只能占三个字节）    handler接受消息对象
	 */
	static void RegisterAckHandler(unsigned char type, unsigned int id, Ack_Handler * handler)
	{
		m_c->GetHandlerMgr()->RegisterAckHandler(type, id, handler);
	}

	/*
	 * 单线程分发消息
	 * return :
	 * param :
	 */
	static void ProcessMessage();

	/*
	 * 单线程获取消息
	 * return :
	 * param :
	 */
	static void MessageFlush();

	/*
	 * 发送消息
	 * return :channelID 通信socket，   message发送的消息
	 * param :
	 */
	static void Send(Safe_Smart_Ptr<Message> &message);

	/*
	 *  绑定组连接
	 * return :key为组里的唯一标识一个channel       group组 id
	 * param : 成功    为返回兵丁的chanlle
	 */
	static int BindGroupChannel(int64 key, int group);

	/*
	 *  解绑定组连接
	 * return :key为组里的唯一标识一个channel       group组 id
	 * param :
	 */
	static void UnBindGroupChannel(int64 key, int group);

	/*
	 * 关闭连接
	 * return：
	 * param：sock连接socket
	 */
	static void CloseChannel(int sock);

	/*
	 * 等待
	 * return：
	 * param：
	 */
	static void Wait();

	/*
	 * 结束等待
	 * return :
	 * param :
	 */
	static void UnWait();

	/*
	 * 停止运行
	 * return :
	 * param :
	 */
	static void Stop();

private:
	static Context *m_c;
	static Event m_e;
};
}

#endif /* NETWORK_MESSAGE_FACADE_H_ */
