/*
 * MysqlInterface.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef SQLINTERFACE_MYSQLINTERFACE_H_
#define SQLINTERFACE_MYSQLINTERFACE_H_

#include <string>
#include <stdarg.h>
#include "SqlConnPool.h"

using namespace std;

namespace CommBaseOut
{

/*
 * 数据库操作接口
 * 自动从连接池获取数据库连接，在对象销毁后，将连接还给连接池
 *执行的sql字串长度不要超过511字符
 */
class CMysqlInterface
{
public:
	CMysqlInterface();
	~CMysqlInterface();

	/*
	 * 释放连接
	 * return :
	 * param :
	 */
	void ReleaseConnection();

	/*
	 * 查询sql语句
	 * return :E_SQLResult
	 * param : sqlStr可格式化语句       ...可变参数
	 */
	int Select(const char *sqlStr,...);

	/*
	 * 执行sql语句,不会从数据库中取出数据
	 * return :E_SQLResult
	 * param :sqlStr可格式化语句       ...可变参数
	 */
	int Execute(const char *sqlStr,...);

	/*
	 * 执行存储过程，并且会返回数据库中的数据（暂时只支持取一个结果集，后面的结果集会被丢弃）
	 * return :E_SQLResult
	 * param :sqlStr可格式化语句       ...可变参数
	 */
	int ExecuteRet(const char *sqlStr,...);

	/*
	 * 获取记录行
	 * return : 记录行
	 * param :
	 */
	int GetRow()
	{
		return m_con->GetRowNum();
	}

	/*
	 * 获取下一条记录
	 * return :
	 * param :
	 */
	void GetNextRow()
	{
		m_con->NextRow();
	}

	/*
	 * 是否还有数据
	 * return :bool
	 * param :
	 */
	bool IsHaveRecord()
	{
		return m_con->HasRowData();
	}

	/*
	 * 根据字段号获取字段数据
	 * return :获取的数据
	 * param :字段号   默认值
	 */
	int GetIntField(unsigned int nField, int nNullValue=0)
	{
		return m_con->GetIntField(nField, nNullValue);
	}
	const char* GetStringField(unsigned int nField, const char* szNullValue="")
	{
		return m_con->GetStringField(nField, szNullValue);
	}
	double GetFloatField(unsigned int nField, double fNullValue=0.0)
	{
		return m_con->GetFloatField(nField, fNullValue);
	}

	/*
	 * 根据字段名获取字段数据
	 * return :获取的数据
	 * param :字段名   默认值
	 */
	int GetIntName(const char *name, int nNullValue=0)
	{
		return m_con->GetIntByName(name,nNullValue);
	}
	int64 GetInt64Name(const char *name, int64 nNullValue=0)
	{
		return m_con->GetInt64ByName(name,nNullValue);
	}
	const char* GetStringName(const char *name, const char* szNullValue="")
	{
		return m_con->GetStringByName(name,szNullValue);
	}
	double GetFloatName(const char *name, double fNullValue=0.0)
	{
		return m_con->GetFloatByName(name,fNullValue);
	}

	/*
	 * 转义 SQL 语句中使用的字符串中的特殊字符
	 * return : 转换的大小
	 * param :　ｄｅｓ 目标缓冲      src 源缓冲    len 源缓冲的需要转换的大小
	 */
	int Escape_string(char * des, char * src, int len)
	{
		return m_con->Escape_string(des, src, len);
	}

private:
	CMysqlConn * m_con;
	char * m_dataBuf;
};
}

#endif /* SQLINTERFACE_MYSQLINTERFACE_H_ */
