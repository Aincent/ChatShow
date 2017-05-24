/*
 * packet.h
 *
 *  Created on: May 23, 2017
 *      Author: root
 */

#ifndef PACKET_H_
#define PACKET_H_
#include "commandlib/alloc.h"

#define PACKET_BY_HEADER_SIZE 9
#define PACKET_BY_BODY_SIZE 10240


typedef struct tcp_packet {
	short _off;
	short cmd;
    char name[3];
    unsigned char cversion;
    unsigned char csubversion;
    unsigned char code;
	char _head[PACKET_BY_HEADER_SIZE + 1];
	char _buf[PACKET_BY_BODY_SIZE + 1];
}tcp_packet;


static inline tcp_packet* create_packet(const short cmd)
{
	tcp_packet* packet = my_new(tcp_packet);
	packet->_off = 0;
	packet->cmd = cmd;
	strcpy(packet->name,"BY");
	packet->cversion = 1;
	packet->csubversion = 1;
	packet->code = 0;
	return packet;
}

static inline void push_packet_int8(tcp_packet* packet,uint8_t vl)
{
	ASSERT(packet->_off + sizeof(uint8_t) <= PACKET_BY_BODY_SIZE);
	packet->_buf[packet->_off++] = (char)vl;
}

static inline void push_packet_int16(tcp_packet* packet,uint16_t vl)
{
	ASSERT(packet->_off + sizeof(uint16_t) <= PACKET_BY_BODY_SIZE);
	hton16(packet->_buf+packet->_off,vl);
	packet->_off += sizeof(uint16_t);
}

static inline void push_packet_int32(tcp_packet* packet,uint32_t vl)
{
	ASSERT(packet->_off + sizeof(uint32_t) <= PACKET_BY_BODY_SIZE);
	hton32(packet->_buf+packet->_off,vl);
	packet->_off += sizeof(uint32_t);
}

static inline void push_packet_int64(tcp_packet* packet,uint64_t vl)
{
	ASSERT(packet->_off + sizeof(uint64_t) <= PACKET_BY_BODY_SIZE);
	hton64(packet->_buf+packet->_off,vl);
	packet->_off += sizeof(uint64_t);
}

static inline void push_packet_str(tcp_packet* packet,const char* vl,int len)
{
	ASSERT(len > 0 && packet->_off + len <= PACKET_BY_BODY_SIZE);
	memcpy(packet->_buf+packet->_off,vl,len);
	packet->_off += len;
}


static inline void push_packet_end(tcp_packet* packet)
{
//	hton16(packet->_head,packet->_off);
	short len = htons(packet->_off + PACKET_BY_HEADER_SIZE  - 2);
	memcpy(packet->_head,&len,2);
	memcpy(packet->_head+2,packet->name,2);
	packet->_head[4] = (char)packet->cversion;
	packet->_head[5] = (char)packet->csubversion;
	hton16(packet->_head+6,packet->cmd);
	packet->_head[8] = (char)packet->code;
}


static inline void destory_packet(tcp_packet* packet)
{
	my_free(packet);
}



#endif /* PACKET_H_ */
