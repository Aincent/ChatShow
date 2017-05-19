/*
 * user_msg.h
 *
 *  Created on: May 18, 2017
 *      Author: root
 */

#ifndef USER_MSG_H_
#define USER_MSG_H_

#include "antlib.h"

int set_serverkey(const char* key,const char* table,int svrid);

#define USERMSG_LOGIN 	TCPMSGID(1,1)

void handle_usermsg_login(handler_msg* msg,void* user_group);
#endif /* USER_MSG_H_ */
