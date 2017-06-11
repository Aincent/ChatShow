/*
 * SqlConnPool.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "SqlConnPool.h"
#include "../Log/Logger.h"

namespace CommBaseOut
{

CSqlConnPool * CSqlConnPool::m_instance = NULL;


CMysqlConn::CMysqlConn():m_mysql(NULL),m_results(NULL),m_record(NULL),m_field(NULL),m_rowNum(0),m_fieldNum(0),m_affectRows(0),m_key(0),m_isFunc(false)
{

}

CMysqlConn::~CMysqlConn()
{
	if(m_results)
	{
		mysql_free_result(m_results);
		m_results = NULL;
	}

	mysql_close(m_mysql);
	m_mysql = NULL;
}

bool CMysqlConn::Init()
{
	m_mysql = mysql_init(NULL);
	if(NULL == m_mysql)
	{
		return false;
	}

	my_bool value = 1;
	if(mysql_options(m_mysql,MYSQL_OPT_RECONNECT,&value))
		return false;

	if(mysql_options(m_mysql,MYSQL_SET_CHARSET_NAME,"utf8"))
		return false;

	if(mysql_options(m_mysql,MYSQL_REPORT_DATA_TRUNCATION,&value))
		return false;

	return true;
}

bool CMysqlConn::Connect(string &host, string &user, string &pwd, string &db, int port, char *socket, int flag)
{
	if(NULL == mysql_real_connect(m_mysql, host.c_str(), user.c_str(), pwd.c_str(), db.c_str(), port, socket, flag))
	{
		return false;
	}

	return true;
}

int CMysqlConn::QuerySQL(string &strSql, bool isFunc)
{
	if(m_results)
	{
		Clear();
	}

	m_isFunc = isFunc;

	return Operate( strSql, true );
}

int CMysqlConn::ExceSQL(string &strSql)
{
//	if(m_results)
	{
		Clear();
	}

	return Operate( strSql, false );
}

int CMysqlConn::Operate(string &strSql, bool hasResults)
{
	if(NULL == m_mysql)
	{
		char dec[512] = {0};
		string what;

		snprintf(dec, 512, "[CSqlConnPool::Operate] : message = [Handle is null], errno = [%d]",eHandleNULL);
		what = dec;

		throw SqlException(what);
	}

	if(mysql_real_query( m_mysql, strSql.c_str(), strSql.size()))
	{
		char dec[512] = {0};
		string what;

		snprintf(dec, 512, "[CSqlConnPool::Operate] : message = [%s], errno = [%d]", mysql_error(m_mysql), eExeError);
		what = dec;

		throw SqlException(what);
	}

	if( hasResults )
	{
		m_results = mysql_store_result(m_mysql);
		if( NULL == m_results )
			return eResError;

		m_rowNum = mysql_num_rows(m_results);
		m_record = mysql_fetch_row(m_results);
		m_field = mysql_fetch_field(m_results);
		m_fieldNum = mysql_num_fields(m_results);

		for(unsigned int i=0; i<m_fieldNum; ++i)
		{
			m_fieldName[m_field[i].name] = i;
		}

		if(m_rowNum == 0)
			return eSeleNULL;
	}
	else
	{
		m_affectRows = mysql_affected_rows(m_mysql);
		if( -1 == m_affectRows )
			return eBefExeError;
	}

	return eSqlSuccess;
}

int64 CMysqlConn::GetInt64Field(unsigned int nField, int64 nNullValue)
{
	if ( NULL == m_results ||
			 (nField + 1 > m_fieldNum ) ||
			  NULL == m_record ||m_record[nField]==NULL)
  	return nNullValue;

	return atoll(m_record[nField]);
}

int CMysqlConn::GetIntField(unsigned int nField, int nNullValue)
{
	if ( NULL == m_results ||
			 (nField + 1 > m_fieldNum ) ||
			  NULL == m_record ||m_record[nField]==NULL)
  	return nNullValue;

	return atoi(m_record[nField]);
}

const char* CMysqlConn::GetStringField(unsigned int nField, const char* szNullValue)
{
	if ( NULL == m_results ||
			 (nField + 1 > m_fieldNum ) ||
			  NULL == m_record ||
			  NULL == m_record[nField] )
		return szNullValue;

	return m_record[nField];
}

double CMysqlConn::GetFloatField(unsigned int nField, double fNullValue)
{
	if ( NULL == m_results ||
		 (nField + 1 > m_fieldNum ) ||
		  NULL == m_record )
	return fNullValue;

	return atof(m_record[nField]);
}

int CMysqlConn::GetIntByName(const char *name, int nNullValue)
{
	string str = name;
	map<string, int>::iterator it = m_fieldName.find(str);
	if(it == m_fieldName.end())
	{
		return nNullValue;
	}

	return GetIntField(it->second, nNullValue);
}

int64 CMysqlConn::GetInt64ByName(const char *name, int64 nNullValue)
{
	string str = name;
	map<string, int>::iterator it = m_fieldName.find(str);
	if(it == m_fieldName.end())
	{
		return nNullValue;
	}

	return GetInt64Field(it->second, nNullValue);
}

const char* CMysqlConn::GetStringByName(const char *name, const char* szNullValue)
{
	string str = name;
	map<string, int>::iterator it = m_fieldName.find(str);
	if(it == m_fieldName.end())
	{
		return szNullValue;
	}

	return GetStringField(it->second, szNullValue);
}

double CMysqlConn::GetFloatByName(const char *name, double fNullValue)
{
	string str = name;
	map<string, int>::iterator it = m_fieldName.find(str);
	if(it == m_fieldName.end())
	{
		return fNullValue;
	}

	return GetFloatField(it->second, fNullValue);
}

bool CMysqlConn::HasRowData()
{
	if( NULL == m_record )
		return false;

	return true;
}

void CMysqlConn::NextRow()
{
	if( NULL == m_results )
		return;

	m_record = mysql_fetch_row(m_results);
}

CSqlConnPool::CSqlConnPool():m_poolSize(0),m_rate(0),m_port(0),m_key(1)
{
}

CSqlConnPool::~CSqlConnPool()
{
}

int CSqlConnPool::AddConnecton(short int count)
{
	for(int i=0; i<count; ++i)
	{
		CMysqlConn * con = NEW CMysqlConn();
		if(con == NULL)
		{
			char dec[512] = {0};
			string what;

			snprintf(dec, 512, "[CSqlConnPool::AddConnecton] : message = [Create connection failed], errno = [%d]",eCreateConError);
			what = dec;

			LOG_BASE(FILEINFO, "AddConnecton failed,error msg[%s]", dec);

			throw SqlException(what);
		}

		if(!con->Init())
		{
			delete con;
			con = NULL;

			char dec[512] = {0};
			string what;

			snprintf(dec, 512, "[CSqlConnPool::AddConnecton] : message = [new connection init failed], errno = [%d]",eHandleNULL);
			what = dec;

			LOG_BASE(FILEINFO, "AddConnecton failed,error msg[%s]", dec);

			throw SqlException(what);
		}

		if(!con->Connect(m_host, m_user, m_pwd, m_db, m_port, NULL, CLIENT_MULTI_STATEMENTS))
		{
			delete con;
			con = NULL;

			char dec[512] = {0};
			string what;

			snprintf(dec, 512, "[CSqlConnPool::AddConnecton] : message = [handle connect failed], errno = [%d]",eConnMysqlError);
			what = dec;

			LOG_BASE(FILEINFO, "AddConnecton failed,error msg[%s]", dec);

			throw SqlException(what);
		}

		unsigned int key = GetKey();
		con->SetKey(key);

		GUARD(CSimLock, objIdle, &m_idleMutex);
		m_idleConn.push_back(con);

		m_poolSize++;
		objIdle.UnLock();
	}

	return eSqlSuccess;
}

int CSqlConnPool::Init(string &host, string &user, string &pwd, string &db, int port, short int size, short int rate)
{
	m_host = host;
	m_user = user;
	m_pwd = pwd;
	m_db = db;
	m_port = port;
	m_rate = rate;

	return AddConnecton(size);
}

CMysqlConn *CSqlConnPool::GetConnection()
{
	GUARD(CSimLock, objIdle, &m_idleMutex);
	int size = m_idleConn.size();
	if(size == 0)
	{
		char dec[128] = {0};
		string what;

		snprintf(dec, 512, "[CSqlConnPool::GetConnection] idle connection is zero");
		what = dec;

		LOG_BASE(FILEINFO, "GetConnection failed,error msg[%s]", dec);

		throw SqlException(what);
	}

	CMysqlConn *con = m_idleConn.front();
	m_idleConn.pop_front();

//	GUARD(CSimLock, obj, &m_usedMutex);
	m_usedConn[con->GetKey()] = con;
	objIdle.UnLock();

	if((size - 1) < m_rate)
	{
		AddConnecton(m_rate);
	}

	return con;
}

void CSqlConnPool::ReleaseConn(unsigned int onlyID)
{
	GUARD(CSimLock, obj, &m_idleMutex);
	map<unsigned int, CMysqlConn *>::iterator it = m_usedConn.find(onlyID);
	if(it == m_usedConn.end())
	{
		char dec[128] = {0};
		string what;

		snprintf(dec, 512, "Failed when Releasing Connection[ID=%u] to pool",onlyID);
		what = dec;

		LOG_BASE(FILEINFO, "ReleaseConn failed,error msg[%s]", dec);

		throw SqlException(what);
	}

	CMysqlConn * con = it->second;
	m_usedConn.erase(it);

//	con->SetKey(0);
	con->Clear();
	m_idleConn.push_back(con);
}
}

