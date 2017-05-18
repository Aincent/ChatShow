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

	char sql[256];
	int n = sprintf(sql,"select max(id) from %s",_g_user_server._table);
	mydb* db = get_mydb(0);
	mydb_recordset* rset = get_recordset(db,sql,n);
	if(rset)
	{
		if(rset->_row_num > 0)
		{
			_g_user_server._maxid = DBFIELD_UINT64(db,rset,0);
			if((int)(_g_user_server._maxid>>32) != _g_user_server._id)
				return -1;
		}
		back_recordset(db,rset);
	}
	_g_user_server._maxid += 1;
	return 0;
}
