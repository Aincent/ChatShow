/*
 * MysqlInterface.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "MysqlInterface.h"
#include "../Log/Logger.h"

namespace CommBaseOut
{

CMysqlInterface::CMysqlInterface():m_con(NULL),m_dataBuf(NULL)
{
	m_con = CSqlConnPool::GetInstance()->GetConnection();
	m_dataBuf = NEW_BASE(char, MYSQL_EXECUTE_BUF);//new char[MYSQL_EXECUTE_BUF];
}

CMysqlInterface::~CMysqlInterface()
{
	ReleaseConnection();

	DELETE_BASE(m_dataBuf, eMemoryArray);
	m_dataBuf = 0;
}

void CMysqlInterface::ReleaseConnection()
{
	if(NULL == m_con)
	{
		char dec[128] = {0};
		string what;

		snprintf(dec, 128, "[CMysqlInterface::ReleaseConnection] Releasing NULL Connection");
		what = dec;

		LOG_BASE(FILEINFO, "release connection failed,error msg[%s]", dec);

		throw SqlException(what);
	}

	CSqlConnPool::GetInstance()->ReleaseConn(m_con->GetKey());
}

int CMysqlInterface::Select(const char *sqlStr,...)
{
//	char log_data[16384] = {0};
	memset(m_dataBuf, 0, MYSQL_EXECUTE_BUF);

	if(NULL == m_con)
	{
		char dec[128] = {0};
		string what;

		snprintf(dec, 128, "[CMysqlInterface::Select] failed select in NULL Connection");
		what = dec;

		LOG_BASE(FILEINFO, "select failed,error msg[%s]", dec);

		throw SqlException(what);
	}

	va_list args;
	va_start(args,sqlStr);
	vsnprintf(m_dataBuf, MYSQL_EXECUTE_BUF - 1, sqlStr, args);
	va_end(args);

	string tmpstr = m_dataBuf;

	return m_con->QuerySQL(tmpstr);
}

int CMysqlInterface::Execute(const char *sqlStr,...)
{
//	char log_data[16384] = {0};
	memset(m_dataBuf, 0, MYSQL_EXECUTE_BUF);

	if(NULL == m_con)
	{
		char dec[128] = {0};
		string what;

		snprintf(dec, 128, "[CMysqlInterface::Execute] failed Execute in NULL Connection");
		what = dec;

		LOG_BASE(FILEINFO, "excute failed,error msg[%s]", dec);

		throw SqlException(what);
	}

	va_list args;
	va_start(args,sqlStr);
	vsnprintf(m_dataBuf, MYSQL_EXECUTE_BUF - 1, sqlStr, args);
	va_end(args);

	string tmpstr = m_dataBuf;

	return m_con->ExceSQL(tmpstr);
}

int CMysqlInterface::ExecuteRet(const char *sqlStr,...)
{
//	char log_data[16384] = {0};
	memset(m_dataBuf, 0, MYSQL_EXECUTE_BUF);

	if(NULL == m_con)
	{
		char dec[128] = {0};
		string what;

		snprintf(dec, 128, "[CMysqlInterface::ExecuteRet] failed Execute in NULL Connection");
		what = dec;

		LOG_BASE(FILEINFO, "ExecuteRet failed,error msg[%s]", dec);

		throw SqlException(what);
	}

	va_list args;
	va_start(args,sqlStr);
	vsnprintf(m_dataBuf, MYSQL_EXECUTE_BUF - 1, sqlStr, args);
	va_end(args);

	string tmpstr = m_dataBuf;

	return m_con->QuerySQL(tmpstr, true);
}
}
