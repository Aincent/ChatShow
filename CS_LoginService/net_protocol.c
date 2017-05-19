/*
 * net_protocol.c
 *
 *  Created on: May 18, 2017
 *      Author: root
 */

#include "net_protocol.h"
#include "user_msg.h"

#define TCPMSG_REGITST_STRING(path,name,msgid,fun) struct pm_tcpmsg_field message[1] = {{"message",0,3,32}};\
	tcpmsg_regist_string(path,name,msgid,message,1,fun);

//check message then add to handler
void do_user_netmsg(handler_msg* msg,uint64_t userid,uint64_t netid)
{
	tcp_stream* s = (tcp_stream*)msg->_data;
	char* name = (char*)(s->_vl->_vl[0]);
	if(name && name[0] && name[1])
	{
		pushmsg_byname((const char*)(s->_vl->_vl[0]),msg);
	}
	else
	{
		close_tcpconnection(netid,"name error");
		free_handler_msg(msg);
	}

}


void init_netprotocol()
{
	// http://url/user/login?name=admin&password=123456&type=1
	TCPMSG_REGITST_STRING("user","login",USERMSG_LOGIN,do_user_netmsg);
}