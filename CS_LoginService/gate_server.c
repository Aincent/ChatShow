/*
 * gate_server.c
 *
 *  Created on: May 24, 2017
 *      Author: root
 */
#include "gate_server.h"
#include "protocollib/tcp_stream.h"
#include "packet.h"
#include "protocollib/tcp_protocol.h"

struct gate_service_config
{
	uint32_t _isuse;
	uint32_t _handlerid;
//	uint32_t _ip;
//	uint32_t _port;
//	uint32_t _type;
	uint64_t _netid;
};

struct gate_service_list
{
	struct gate_service_config _list[GATE_SERVICER_NUM];
};


struct gate_service_list* _g_server_list = NULL;

void gate_server_init(int handler_group)
{
	ASSERT(_g_server_list == NULL);
	_g_server_list = (struct gate_service_list*)my_malloc(sizeof(struct gate_service_list));
	int i = 0;
	for(; i < GATE_SERVICER_NUM; i++)
	{
		struct gate_service_config* g  = &(_g_server_list->_list[i]);
		g->_handlerid = register_handler(handler_group,"gateserver_01",gate_service_domsg,gate_service_doproc,NULL,g);
		ASSERT(g->_handlerid > 0);
	}
}


void __push_gate_server_netid(uint32_t _handlerid,uint64_t netid)
{
	ASSERT(_g_server_list != NULL);
	int i = 0;
	for(; i < GATE_SERVICER_NUM ; i++)
	{
		struct gate_service_config* g  = &(_g_server_list->_list[i]);
		if(g->_isuse == 1 && g->_handlerid == _handlerid)
		{
			g->_netid = netid;
			break;
		}
	}
}

uint64_t _pop_gate_server_netid()
{
	ASSERT(_g_server_list != NULL);
	int i = 0;
	for(; i < GATE_SERVICER_NUM ; i++)
	{
		struct gate_service_config* g  = &(_g_server_list->_list[i]);
		if(g->_isuse == 1)
		{
			return g->_netid;
		}
	}
}

void*  _pop_gate_server_handlerid()
{
	ASSERT(_g_server_list != NULL);
	int i = 0;
	for(; i < GATE_SERVICER_NUM ; i++)
	{
		struct gate_service_config* g  = &(_g_server_list->_list[i]);
		if(g->_isuse == 0)
		{
			return g;
		}
	}

	return 0;
}

void gate_service_domsg(handler_msg* msg,void* g,uint32_t hid)
{
	switch(msg->_msgid)
	{
	case TCPCONNECT_MSGID:
		{
			struct tcp_connect_data* data = (struct tcp_connect_data*)msg->_data;
			__push_gate_server_netid(data->_user_handlerid, msg->_userid);
			printf("connect success .......netid = %lld _handlerid = %d \n", msg->_userid,data->_user_handlerid);

			tcp_packet* packet = create_packet(CONN_GATE_INFO);
			push_packet_int32(packet,3305);
			push_packet_end(packet);
			handler_msg* hmsg = create_handler_msg(packet->_off);
			hmsg->_datalen = packet->_off;
			memcpy(hmsg->_data,packet->_buf,packet->_off);
			destory_packet(packet);
			tcp_sendmsg(msg->_userid,hmsg);

//			free_handler_msg(msg);
			break;
		}
	case TCPCONNECT_LOST_MSGID:
		{
			printf("connect error ... %d \n", hid);
			free_handler_msg(msg);
			break;
		}
	case CONN_GATE_INFO:
		{
			tcp_stream* info = (tcp_stream*)msg->_data;
			struct gate_service_config* _g_server_config = (struct gate_service_config*)g;
			printf("Hello ....msgid = %d  netid = %lld _handlerid = %d  hid = %d \n",msg->_msgid, info->_netid,_g_server_config->_handlerid,hid);
			int port = ntoh32(info->_buf);
			int len = ntoh32(info->_buf+4);
			char ip[32] = {0};
			memcpy(ip,info->_buf+8,len);

			printf("%d \n", port);
			printf("%d \n", len);
			printf("%s \n", ip);

//			free_handler_msg(msg);
			break;
		}
	}
}

void gate_service_doproc(void* hdata,uint32_t hid)
{

}


uint32_t gate_server_handlerid(uint64_t netid)
{
	ASSERT(_g_server_list != NULL);
	int i = 0;
	for(; i < GATE_SERVICER_NUM ; i++)
	{
		struct gate_service_config* g  = &(_g_server_list->_list[i]);
		if(g->_isuse == 1 && g->_netid == netid)
		{
			return g->_handlerid;
		}
	}

	return 0;
}



int gate_service_conn(int svrid,const char* ip,int port,int protocol_type)
{
	struct gate_service_config* g = _pop_gate_server_handlerid();
	if(g == 0)
		return -1;

	uint32_t addr = INADDR_ANY;
	if (ip&&ip[0]) {addr=inet_addr(ip);}

	tcp_packet* packet = create_packet(CONN_GATE_INFO);
	push_packet_int32(packet,3305);
	push_packet_end(packet);
	g->_isuse = 1;
	conn_tcpprotocol(svrid,addr,port,packet->_buf,packet->_off,0,g->_handlerid,protocol_type);
	destory_packet(packet);
	return 0;
}

void gate_service_call(char* message,int len)
{
	ASSERT(_g_server_list != NULL);

	tcp_packet* packet = create_packet(CONN_GATE_INFO);
	push_packet_int32(packet,3305);
	push_packet_int32(packet,len);
	push_packet_str(packet,message,len);
	push_packet_end(packet);

	handler_msg* hmsg = create_handler_msg(packet->_off);
	hmsg->_datalen = packet->_off;
	memcpy(hmsg->_data,packet->_buf,packet->_off);

	destory_packet(packet);

	tcp_sendmsg(_pop_gate_server_netid(),hmsg);
}
