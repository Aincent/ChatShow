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


///////////////////////////////////////////////////////////////////////////
void handle_usermsg_login(handler_msg* msg,void* user_group)
{
	char* message = (char*)MSGINFO_FIELD(msg,0);
	tcp_stream* info = (tcp_stream*)msg->_data;
	struct user_account* user = _load_user_cache(name,user_group);
	int result,n;
	uint32_t t = get_mytime();
	char pswd[128]="";
	//0 OK,1 password error,2 user limit used,3 user not exist
	if(user->_state == -1)
	{
		result = 3;
	} else if(user->_time > t) {
		result = 2;
		t = user->_time - t;
	} else {
		MYMD5_ENCRYPT2((uint8_t*)user->_name,strlen(user->_name),(uint8_t*)password,strlen(password),pswd);
		if(memcmp(user->_password,pswd,32) == 0)
		{
			n = sprintf(pswd,"%s:%lu:%u",_g_user_server._key,user->_id,t);
			MD5((uint8_t*)pswd,n,pswd);
			result = 0;
		} else { result = 1; }
	}
	//
	if(result == 0)
	{
		info->_len = sprintf(info->_buf,"{\"msg\":%d,\"result\":0,\"id\":%lu,\"key\":\"%s\",\"time\":%u}",
				USERMSG_LOGIN_BACK,user->_id,pswd,t);
	}
	else
	{
		info->_len = sprintf(info->_buf,"{\"msg\":%d,\"result\":%d,\"id\":0,\"key\":\"\",\"time\":%u}",
				USERMSG_LOGIN_BACK,result,result==2?t:0);
	}
	msg->_msgid = 0;
	USERLOG("%s [login]name=%s,result=%d,ip=%s",get_log_date(),user->_name,result,to_strip(info->_ip));
	tcp_sendmsg(info->_netid,msg);
}
