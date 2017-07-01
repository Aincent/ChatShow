/*
 * net_packet.h
 *
 *  Created on: Jun 16, 2017
 *      Author: root
 */

#ifndef NET_PACKET_H_
#define NET_PACKET_H_

#include "commandlib/alloc.h"

#define HEADER_LENGTH 26
#define BODY_LENGTH 10240

typedef struct sPacket
{
	int length;
	unsigned char security;
	unsigned char remoteType;
	short int remoteID;
	unsigned char localType; //用于标识，是什么类型的端发送的数据
	unsigned int messageID;
	unsigned char messageType;
	int reqID;
	unsigned long long sendTime;
	short _off;
	char _buf[BODY_LENGTH + 1];
}net_packet;


static inline net_packet* create_net_packet(int messageID,int messageType)
{
	net_packet* packet = my_new(net_packet);
	packet->_off = HEADER_LENGTH;
	packet->messageID = messageID;
	packet->messageType = messageType;
	packet->remoteType = 1;
	packet->remoteID = 1;
	packet->localType = 0;//6
	packet->security = 0;
	packet->reqID = 0;
	packet->sendTime = 0;

	return packet;
}

static inline void push_net_packet_int8(net_packet* packet,uint8_t vl)
{
	ASSERT(packet->_off + sizeof(uint8_t) <= BODY_LENGTH);
	packet->_buf[packet->_off++] = (char)vl;
}

static inline void push_net_packet_int16(net_packet* packet,uint16_t vl)
{
	ASSERT(packet->_off + sizeof(uint16_t) <= BODY_LENGTH);
	hton16(packet->_buf+packet->_off,vl);
	packet->_off += sizeof(uint16_t);
}

static inline void push_net_packet_int32(net_packet* packet,uint32_t vl)
{
	ASSERT(packet->_off + sizeof(uint32_t) <= BODY_LENGTH);
	hton32(packet->_buf+packet->_off,vl);
	packet->_off += sizeof(uint32_t);
}

static inline void push_net_packet_int64(net_packet* packet,uint64_t vl)
{
	ASSERT(packet->_off + sizeof(uint64_t) <= BODY_LENGTH);
	hton64(packet->_buf+packet->_off,vl);
	packet->_off += sizeof(uint64_t);
}

static inline void push_net_packet_str(net_packet* packet,const char* vl,int len)
{
	ASSERT(len > 0 && packet->_off + len <= BODY_LENGTH);
	memcpy(packet->_buf+packet->_off,vl,len);
	packet->_off += len;
}

static inline void push_net_packet_end(net_packet* packet)
{
	packet->length = (packet->_off - HEADER_LENGTH);
	int len = packet->length;
	hton32(packet->_buf,len);
//	memcpy(packet->_buf,&len,4);
	packet->_buf[4] = (char)packet->security;
	packet->_buf[5] = (char)packet->remoteType;
	hton16(packet->_buf+6,packet->remoteID);
	packet->_buf[8] = (char)packet->localType;
	hton32(packet->_buf+9,packet->messageID);
	packet->_buf[13] = (char)packet->messageType;
	hton32(packet->_buf+14,packet->reqID);
	hton32(packet->_buf+18,packet->sendTime);
}

static inline void destory_net_packet(net_packet* packet)
{
	my_free(packet);
}



#endif /* NET_PACKET_H_ */
