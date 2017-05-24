/*
 * gate_server.c
 *
 *  Created on: May 24, 2017
 *      Author: root
 */
#include "gate_server.h"
#include "protocollib/tcp_stream.h"

struct gate_service_config
{
	uint32_t _handlerid;
	uint64_t _netid;
};

struct gate_service_config* _g_server_config = NULL;

void gateserver_init(int handler_group)
{
	ASSERT(_g_server_config == NULL);
	_g_server_config = (struct gate_service_config*)my_malloc(sizeof(struct gate_service_config));
	_g_server_config->_handlerid = register_handler(handler_group,"gateserver_01",gate_service_domsg,gate_service_doproc,NULL,NULL);
	ASSERT(_g_server_config->_handlerid > 0);
}

void gate_service_domsg(handler_msg* msg,void* g,uint32_t hid)
{
	switch(msg->_msgid)
	{
	case GATEMSG_INFO:
//		handle_usermsg_login(msg,g);
		{
			tcp_stream* info = (tcp_stream*)msg->_data;
			printf("Hello ....msgid = %d  netid = %lld \n",msg->_msgid, info->_netid);
			break;
		}
	}
}

void gate_service_doproc(void* hdata,uint32_t hid)
{

}

int gateserver_handlerid()
{
	ASSERT(_g_server_config != NULL);
	return _g_server_config->_handlerid;
}
