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
		char sql[256];
		int n = snprintf(sql,sizeof(sql),"select psd,state,id from %s where name='%s'",_g_user_server._table,name);

		mydb* db = get_mydb(0);
		mydb_recordset* rset = get_recordset(db,sql,n);
		if(rset)
		{
			if(rset->_row_num > 0)
			{
				user = get_user_cache(g);
				strncpy(user->_name,name,33);
				user->_state = -1;

				strncpy(user->_password,DBFIELD_STRING(db,rset,0),32);
				user->_state = (char)DBFIELD_INT(db,rset,1);
				user->_id =  DBFIELD_UINT64(db,rset,2);

				add_user(user,g);
			}
			back_recordset(db,rset);
		}

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
		printf("%s \n",message);
		user->_netid = info->_netid;
		gate_service_call(message,strlen(message) +1);
	}
	else
	{
		char mess[33] = {0};
		strncpy(mess,message,33);
		info->_len = sprintf(info->_buf,"{\"%s\"",mess);
		msg->_msgid = 1025;

		tcp_sendmsg(info->_netid,msg);
	};
}


void handle_usermsg_register(handler_msg* msg,void* user_group)
{
	char* name = (char*)MSGINFO_FIELD(msg,0);
	char* password = (char*)MSGINFO_FIELD(msg,1);
	int result = 1;
	char pswd[33];
	tcp_stream* info = (tcp_stream*)msg->_data;

	char sql[256];
	int n = snprintf(sql,sizeof(sql),"select psd,state,id from %s where name='%s'",_g_user_server._table,name);

	mydb* db = get_mydb(0);
	mydb_recordset* rset = get_recordset(db,sql,n);
	if(rset)
	{
		if(rset->_row_num > 0)
		{
			result = 0;
		}
		back_recordset(db,rset);
	}

	if(result == 1)
	{
		MYMD5_ENCRYPT2((uint8_t*)name,strlen(name),(uint8_t*)password,strlen(password),pswd);
		int n = snprintf(sql,sizeof(sql),"insert into %s (name,psd,state) values (\'%s\',\'%s\',0)",
				_g_user_server._table,name,pswd);

		mydb* db = get_mydb(0);
		if(0 == do_sql(db,sql,n))
		{
			result = 0;
		}
	}


	info->_len = sprintf(info->_buf,"{\"result\":%d}",result);
	msg->_msgid = 0;
	tcp_sendmsg(info->_netid,msg);
}

void handle_usermsg_password(handler_msg* msg,void* user_group)
{
	char* name = (char*)MSGINFO_FIELD(msg,0);
	char* password = (char*)MSGINFO_FIELD(msg,1);
	char* newpassword = (char*)MSGINFO_FIELD(msg,2);
	char pswd[33];
	int result = 1;
	tcp_stream* info = (tcp_stream*)msg->_data;
	MYMD5_ENCRYPT2((uint8_t*)name,strlen(name),(uint8_t*)password,strlen(password),pswd);

	char sql[256];
	MYMD5_ENCRYPT2((uint8_t*)name,strlen(name),(uint8_t*)newpassword,strlen(newpassword),pswd);
	int n = snprintf(sql,sizeof(sql),"update %s set psd=\'%s\' where name=\'%s\';",_g_user_server._table,pswd,name);
	mydb* db = get_mydb(0);
	if(0 == do_sql(db,sql,n))
	{
		result = 0;
	}

	info->_len = sprintf(info->_buf,"{\"result\":%d}",result);
	msg->_msgid = 0;
	tcp_sendmsg(info->_netid,msg);
}
