/*
 * user_msg.c
 *
 *  Created on: May 18, 2017
 *      Author: root
 */
#include "user_msg.h"
#include "server_db.h"
#include "user_list.h"

struct user_server
{
	char _key[32];
	char _table[32];
	int _id;
	uint64_t _maxid;
};

struct user_server _g_user_server = {"","",0,0};
