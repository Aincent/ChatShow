/*
 * gate_server.h
 *
 *  Created on: May 24, 2017
 *      Author: root
 */

#ifndef GATE_SERVER_H_
#define GATE_SERVER_H_
#include "antlib.h"

#define GATE_SERVICER_NUM 10
#define CONN_GATE_INFO 	1000

void gate_service_domsg(handler_msg* msg,void* g,uint32_t hid);

void gate_service_doproc(void* hdata,uint32_t hid);

void gate_server_init(int handler_group);

uint32_t gate_server_handlerid();

int gate_service_conn(int svrid,const char* ip,int port,int protocol_type);

void gate_service_call(uint64_t charid,char* name);

#endif /* GATE_SERVER_H_ */
