/*
 * net_protocol.c
 *
 *  Created on: May 18, 2017
 *      Author: root
 */

#include "net_protocol.h"
#include "antlib.h"
#include "user_msg.h"
#include "user_list.h"
#include "gate_server.h"
#include "protocollib/tcp_protocol.h"

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

void do_gate_netmsg(handler_msg* msg,uint64_t userid,uint64_t netid)
{
	uint32_t handlerid = gate_server_handlerid(netid);
	if(handlerid > 0)
		send_handler_msg(gate_server_handlerid(netid),msg);
	else
		write_log("do_gate_netmsg handlerid = %d error",handlerid);
}

void init_netprotocol()
{
	// http://url/user/login?name=admin&password=123456&type=1

//	TCPMSG_REGITST_STRING("user","login",USERMSG_LOGIN,do_user_netmsg);
	struct pm_tcpmsg_field message[1] = {{"message",0,3,33}};
	tcpmsg_regist_string("user","login",USERMSG_LOGIN,message,1,do_user_netmsg);

	struct pm_tcpmsg_field signup[2] = {{"name",0,3,33},{"password",0,6,33}};
	tcpmsg_regist_string("user","register",USERMSG_REGISTER,signup,2,do_user_netmsg);

	struct pm_tcpmsg_field password[3] = {{"name",0,3,33},{"password",0,6,33},{"newpassword",0,6,33}};
	tcpmsg_regist_string("user","password",USERMSG_PASSWORD,password,3,do_user_netmsg);

	tcpmsg_regist_binary(CONN_GATE_INFO,NULL,0,do_gate_netmsg);
	tcpmsg_regist_binary(TCPCONNECT_MSGID,NULL,0,do_gate_netmsg);
	tcpmsg_regist_binary(TCPCONNECT_LOST_MSGID,NULL,0,do_gate_netmsg);


}
