/*
 * NetWorkConfig.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef NETWORK_NETWORKCONFIG_H_
#define NETWORK_NETWORKCONFIG_H_

#include <string>
#include "../define.h"
#include "InetAddress.h"
#include "../Smart_Ptr.h"
#include "../util.h"
#include "../Memory/MemAllocator.h"

using namespace std;

namespace CommBaseOut
{

enum EMessageType
{
	ConnMessage=0,
	SimpleMessage,
	Request,
	Ack,
	SystemMessage
};

enum ESystemErr
{
	eConnFailMessage=0,
	eConnErrMessage,
	eConnBuildSuccess
};

struct SystemMessageCon
{
	int channel;
	short int lid;
	unsigned char ltype;
	short int rid;
	unsigned char rtype;
	int err;
	char ip[16];
	short int port;
};

enum ConnMessageType
{
	C2SMessage=0,//c发送到s 的连接包
	S2CMessage,//s发送到c 的连接包
	C2SGroupMessage,//连接组消息
	S2CGroupMessage//连接组消息
};

enum E_NetWorkRet
{
	eNetSuccess=0,
	eCreateSockErr,//创建socket错误
	eBindErr, //綁定错误
	eListenErr, //监听错误
	eConnErr, //连接远程错误
	eAcceptErr, //接受连接错误
	eLimitFdErr, //设置资源限制出错
	eCreateEpollErr, //创建epoll错误
	eGetSocketErr, // 获取socket设置错误
	eSetSocketErr, // 设置socket错误
	eCtlEpollErr, //注册epoll事件错误
	eAddSeTimeoutErr, //增加连接超时错误
	eSessionAdded, //连接已经添加过
	eNotTimeOut, //没有设置连接超时
	eNoEnughWrite, //没有足够的空间写入
	eWriteHeaderErr, //写入包tou错误
	eWritePacketErr, //写入包体错误
	eWriteBuffErr, //写入一个buff错误
	eBuffNULL, // buff为空
	eReadHeaderErr, //读出包tou错误
	eReadPacketErr, //读出包体错误
	ePakcetLenErr=21, //包长度异常
	eSocketErr, // sokcet异常
	eSocketClose, //socket断开
	eNetError, //网络错误
	eCopyError, //拷贝错误
	eTokenErr, //请求token错误
	eReqTimeOut,//请求超时
	eConMessageErr, //连接包异常
	eSockSendErr, //socket发送错误
	eEpollClose, //epoll异常事件
	eEpollNull, //epoll空事件
	eSessionTimeout, // Session超时
	eConnPacketErr, //连接包错误
	eSessionError,//session错误
	eInitDispatchErr, //初始化dispatch错误
	eGroupContinue, // 组连接成功
};

enum E_EpollType
{
	eAcceptEpoll=0,
	eConnEpoll
};

enum E_ConsoleType
{
	eClient=0,
	eGateServer,
	eGameServer,
	ePathServer,
	eWorldServer,
	eDBServer,
	eLoginServer,
	eManagerServer,
	eWebServer,
	eMax
};

#pragma pack(push,1)
typedef struct sHeader
{
	int length;
	bool security;

	unsigned char remoteType;
	short int remoteID;
	unsigned char localType; //用于标识，是什么类型的端发送的数据

	DWORD messageID;
	BYTE messageType;
	int reqID;

	DWORD64 sendTime;

	sHeader():length(0),security(false),remoteType(0),remoteID(-1),localType(0),messageID(-1),messageType(0),reqID(0),sendTime(0)
	{

	}

	void toSmallEndian()
	{
		if(remoteType > 0)
			return;

		length = htonl(length);
		remoteID = htons(remoteID);
		messageID = htonl(messageID);
		reqID = htonl(reqID);
		sendTime = CUtil::ntohll(sendTime);
	}

	void toBigEndian()
	{
		if(localType > 0)
			return;

		length = ntohl(length);
		remoteID = ntohs(remoteID);
		messageID = ntohl(messageID);
		reqID = ntohl(reqID);
		sendTime = CUtil::ntohll(sendTime);
	}

	void toBigEndianEx()
	{
		length = ntohl(length);
		remoteID = ntohs(remoteID);
		messageID = ntohl(messageID);
		reqID = ntohl(reqID);
		sendTime = CUtil::ntohll(sendTime);
	}
}packetHeader;

const int HEADER_LENGTH = sizeof(packetHeader);

#pragma pack(pop)

struct ChannelConfig
{
	short int local_id_;
	unsigned char local_type_;

	//毫秒，每个channel多久没收到连接包就断开
	int channel_keep_time_;
	bool security_;

	//int send_buffer_size_;
	//int recv_buffer_size_;

	ChannelConfig():local_id_(0),local_type_(0),channel_keep_time_(0),security_(false)
	{
	}

	virtual ~ChannelConfig()
	{

	}
};

struct AcceptorConfig : public ChannelConfig
{
	string         listen_ip_;
	unsigned short listen_port_;
	size_t max_allow_accept_;

	int aes_algorithm_type_;

	bool is_packet_keepalive;

	int recv_packet_overfllower_count_;

	//int recv_packet_overtime_timer_;

	AcceptorConfig():listen_port_(0),max_allow_accept_(10000),aes_algorithm_type_(128),is_packet_keepalive(false)
	,recv_packet_overfllower_count_(0)//,recv_packet_overtime_timer_(0)
	{
	}

	AcceptorConfig * operator = (AcceptorConfig & a)
	{
		local_id_ = a.local_id_;
		local_type_ = a.local_type_;
		channel_keep_time_ = a.channel_keep_time_;
		security_ = a.security_;
		listen_ip_ = a.listen_ip_;
		listen_port_ = a.listen_port_;
		max_allow_accept_ = a.max_allow_accept_;
		aes_algorithm_type_ = a.aes_algorithm_type_;
		is_packet_keepalive = a.is_packet_keepalive;
		recv_packet_overfllower_count_ = a.recv_packet_overfllower_count_;

		return this;
	}
};

struct ConnectionConfig : public ChannelConfig
{
	Safe_Smart_Ptr<Inet_Addr> remote_address_;

	short int remote_id_;
	unsigned char remote_type_;

	int aes_algorithm_type_;

	bool re_connect_;
	unsigned int check_wait_time_;
	BYTE group_count; //组数量， 默认0为不建立，大于0表示组连接的数目

	ConnectionConfig():remote_id_(0),remote_type_(0),aes_algorithm_type_(128),re_connect_(false),check_wait_time_(0),group_count(0)
	{
		remote_address_ = NEW Inet_Addr();
	}
};

enum requeset_type
{
	eRequestClose=0,
	eRequestListen,
	eRequestConn,
	eRequestMax,
};

struct request_close
{
	int id;
	DWORD opaque;
};

struct request_listen
{
	int fd;
	WORD opaque;
	char host[16];
	BYTE index;
};

struct request_conn
{
	int fd;
	BYTE index;
};

struct request_package
{
	BYTE header[8];
	union
	{
		char buffer[256];
		struct request_close close;
		struct request_listen listen;
		struct request_conn conn;
	} u;
	BYTE dummy[256];

	request_package()
	{
		memset(header, 0, 8);
		memset(dummy, 0, 256);
	}
};

}

#endif /* NETWORK_NETWORKCONFIG_H_ */
