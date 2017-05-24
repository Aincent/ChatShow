/*
 * gate_server.h
 *
 *  Created on: May 24, 2017
 *      Author: root
 */

#ifndef GATE_SERVER_H_
#define GATE_SERVER_H_
#include "antlib.h"

#define GATEMSG_INFO 	0x103

void gate_service_domsg(handler_msg* msg,void* g,uint32_t hid);

void gate_service_doproc(void* hdata,uint32_t hid);

void gateserver_init(int handler_group);

int gateserver_handlerid();

#endif /* GATE_SERVER_H_ */
