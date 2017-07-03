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
#include "user_list.h"
#include "net_packet.h"

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

	return 0;
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
			printf("connect success .......netid = %ld _handlerid = %d \n", msg->_userid,data->_user_handlerid);

//			tcp_packet* packet = create_packet(CONN_GATE_INFO);
//			push_packet_int32(packet,3305);
//			push_packet_end(packet);
//			handler_msg* hmsg = create_handler_msg(packet->_off);
//			hmsg->_datalen = packet->_off;
//			memcpy(hmsg->_data,packet->_buf,packet->_off);
//			destory_packet(packet);
//			tcp_sendmsg(msg->_userid,hmsg);

//			free_handler_msg(msg);
//			char s[] = "hello World !";
//			gate_service_call(s,strlen(s));
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
			printf("Hello ....msgid = %d  netid = %ld _handlerid = %d  hid = %d \n",msg->_msgid, info->_netid,_g_server_config->_handlerid,hid);
			int code = normal_pop_tcpstream_int32(info);
			int port = normal_pop_tcpstream_int32(info);
			uint16_t localID = normal_pop_tcpstream_int16(info);
			int len = normal_pop_tcpstream_int32(info);
			char ip[32] = {0};
			pop_tcpstream_str(info,ip,len);
			len = normal_pop_tcpstream_int32(info);
			char name[32] = {0};
			pop_tcpstream_str(info,name,len);

			printf("code = %d \n",code);
			printf("name = %s \n", name);
			printf("port = %d \n", port);
			printf("localID = %d \n", localID);
			printf("%d \n", len);
			printf("ip = %s \n", ip);
			uint64_t charid = 0;
			uint64_t netid = pop_user_netid_byname(name,&charid);
			printf("charid =  %ld \n", charid);
			if(netid > 0)
			{
				info->_len = sprintf(info->_buf,"{\"msg\":%s,\"charid\":%ld,\"ip\":%s,\"port\":%d,\"localid\":%d}",name,(((uint64_t)1 << 48) | charid),ip,port,localID);
				msg->_msgid = 0;
				tcp_sendmsg(netid,msg);
			}

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

//	tcp_packet* packet = create_packet(CONN_GATE_INFO);
//	push_packet_int32(packet,3305);
//	push_packet_end(packet);

	net_packet* packet = create_net_packet(0,0);
	push_net_packet_int8(packet,6);
	push_net_packet_int16(packet,1);
	push_net_packet_end(packet);

	g->_isuse = 1;
	conn_tcpprotocol(svrid,addr,port,packet->_buf,packet->_off,0,g->_handlerid,protocol_type);
	destory_net_packet(packet);
	return 0;
}

void gate_service_call(uint64_t charid,char* name)
{
	ASSERT(_g_server_list != NULL);
	uint64_t netid = _pop_gate_server_netid();
	if(netid <= 0)
		return;
//	tcp_packet* packet = create_packet(CONN_GATE_INFO);
//	push_packet_int32(packet,3305);
//	push_packet_int32(packet,len);
//	push_packet_str(packet,message,len);
//	push_packet_end(packet);

	net_packet* packet = create_net_packet(CONN_GATE_INFO,2);
	push_net_packet_int64(packet,charid);
	push_net_packet_int32(packet,strlen(name));
	push_net_packet_str(packet,name,strlen(name));
	push_net_packet_end(packet);

	handler_msg* hmsg = create_handler_msg(packet->_off);
	hmsg->_datalen = packet->_off;
	memcpy(hmsg->_data,packet->_buf,packet->_off);

	destory_net_packet(packet);

	tcp_sendmsg(netid,hmsg);
}
