/*
 * user_msg.c
 *
 *  Created on: May 18, 2017
 *      Author: root
 */
#include "user_msg.h"
#include "server_db.h"
#include "user_list.h"
#include "gate_server.h"

struct user_server
{
	char _key[32];
	char _table[32];
	int _id;
	uint64_t _maxid;
};

struct user_server _g_user_server = {"","",0,0};


int set_serverkey(const char* key,const char* table,int svrid)
{
	memset(&_g_user_server,0,sizeof(_g_user_server));
	strncpy(_g_user_server._key,key,31);
	strncpy(_g_user_server._table,table,31);
	_g_user_server._id = svrid;

//	char sql[256];
//	int n = sprintf(sql,"select max(id) from %s",_g_user_server._table);
//	mydb* db = get_mydb(0);
//	mydb_recordset* rset = get_recordset(db,sql,n);
//	if(rset)
//	{
//		if(rset->_row_num > 0)
//		{
//			_g_user_server._maxid = DBFIELD_UINT64(db,rset,0);
//			if((int)(_g_user_server._maxid>>32) != _g_user_server._id)
//				return -1;
//		}
//		back_recordset(db,rset);
//	}

	_g_user_server._maxid += 1;
	return 0;
}

struct user_account* _load_user_cache(char* name,void* g)
{
	to_smallwords(name);
	struct user_account* user = get_user_byname(name,g);
	if(NULL == user)
	{
		user = get_user_cache(g);
		strncpy(user->_name,name,31);
		user->_state = -1;

//		char sql[256];
//		int n = snprintf(sql,sizeof(sql),"select pswd,state,id from %s where name='%s'",_g_user_server._table,name);
//
//		mydb* db = get_mydb(0);
//		mydb_recordset* rset = get_recordset(db,sql,n);
//		if(rset)
//		{
//			if(rset->_row_num > 0)
//			{
//				strncpy(user->_password,DBFIELD_STRING(db,rset,0),32);
//				user->_state = (char)DBFIELD_INT(db,rset,1);
//				user->_id =  DBFIELD_UINT64(db,rset,2);
//			}
//			back_recordset(db,rset);
//		}

		add_user(user,g);
	}
	return user;
}

///////////////////////////////////////////////////////////////////////////

void handle_usermsg_login(handler_msg* msg,void* user_group)
{
	char* message = (char*)MSGINFO_FIELD(msg,0);
	tcp_stream* info = (tcp_stream*)msg->_data;
	struct user_account* user = _load_user_cache(message,user_group);
	if(user)
	{
		user->_netid = info->_netid;
		gate_service_call(info->_buf,info->_len);
	}
	else
	{
		char mess[32] = {0};
		strncpy(mess,message,31);
		info->_len = sprintf(info->_buf,"{\"%s\"",mess);
		msg->_msgid = 1025;

		tcp_sendmsg(info->_netid,msg);
	};
}
