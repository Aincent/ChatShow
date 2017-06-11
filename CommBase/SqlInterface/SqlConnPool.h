/*
 * SqlConnPool.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef SQLINTERFACE_SQLCONNPOOL_H_
#define SQLINTERFACE_SQLCONNPOOL_H_

#include <mysql/mysql.h>
#include <string>
#include "../Common.h"
#include <list>
#include <map>
#include "../define.h"
#include "../Memory/MemAllocator.h"

using namespace std;

namespace CommBaseOut
{

enum E_SQLResult
{
	eSqlSuccess=0, //成功
	eSeleNULL, //查询结果为0个数据
	eHandleNULL, //数据库连接句柄为空
	eExeError, //执行错误
	eResError, //返回结果错误
	eBefExeError, //上一次执行失败
	eCreateConError, //新建连接错误
	eConnMysqlError, //连接数据库失败
};

/*
 * 数据库抛出的异常
 */
class SqlException : public std::exception
{
public :
	SqlException(string &dec):m_what(dec){}
	virtual ~SqlException() throw()
	{}

	virtual const char* what() const throw()
	{
		return m_what.c_str();
	}

private:
	string m_what;
};


/*
 *单个连接，封装sql操作
 */
class CMysqlConn
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:
	CMysqlConn();
	~CMysqlConn();

	/*
	 * 初始化数据库连接
	 * return :
	 * param :
	 */
	bool Init();

	/*
	 * 连接数据库
	 * return : 主机名或者ip   库用户名      用户密码    数据库名     端口    设置采用套接字或命名管道   标志设置
	 * param :
	 */
	bool Connect(string &host, string &user, string &pwd, string &db, int port = 0, char *socket = NULL, int flag = 0);

	/*
	 * 查询数据
	 * return :E_SQLResult
	 * param :要执行的语句
	 */
	int QuerySQL(string &strSql, bool isFunc = false);

	/*
	 * 执行语句
	 * return :E_SQLResult
	 * param :要执行的语句
	 */
	int ExceSQL(string &strSql);

	/*
	 * 返回记录数
	 * return :记录数
	 * param :
	 */
	unsigned int GetRowNum()
	{
		return m_rowNum;
	}

	/*
	 * 返回字段数
	 * return :字段数
	 * param :
	 */
	unsigned int GetFieldNum()
	{
		return m_fieldNum;
	}

	/*
	 * 返回影响的行
	 * return :影响的行
	 * param :
	 */
	int GetAffectNum()
	{
		return m_affectRows;
	}

	/*
	 * 根据字段号获取字段数据
	 * return :获取的数据
	 * param :字段号   默认值
	 */
	int GetIntField(unsigned int nField, int nNullValue=0);
	int64 GetInt64Field(unsigned int nField, int64 nNullValue=0);
	const char* GetStringField(unsigned int nField, const char* szNullValue="");
	double GetFloatField(unsigned int nField, double fNullValue=0.0);

	/*
	 * 根据字段名获取字段数据
	 * return :获取的数据
	 * param :字段名   默认值
	 */
	int GetIntByName(const char *name, int nNullValue=0);
	int64 GetInt64ByName(const char *name, int64 nNullValue=0);
	const char* GetStringByName(const char *name, const char* szNullValue="");
	double GetFloatByName(const char *name, double fNullValue=0.0);


	/*
	 * 转义 SQL 语句中使用的字符串中的特殊字符
	 * return : 转换的大小
	 * param :　ｄｅｓ 目标缓冲      src 源缓冲    len 源缓冲的需要转换的大小
	 */
	int Escape_string(char * des, char * src, int len)
	{
		return mysql_real_escape_string(m_mysql, des, src, len);
	}


	/*
	 *是否有数据
	 *return :true 有    false 没有
	 *param :
	 */
	bool HasRowData();


	/*
	 * 获取下一条记录
	 * return :
	 * param :
	 */
	void NextRow();

	/*
	 * 设置和获取唯一标识
	 * return :
	 * param :
	 */
	void SetKey(unsigned int key)
	{
		m_key = key;
	}
	unsigned int GetKey()
	{
		return m_key;
	}

	/*
	 * 恢复状态
	 * return :
	 * param :
	 */
	void Clear()
	{
		if(m_results)
		{
			mysql_free_result(m_results);
		m_results = NULL;
		}

		if(m_isFunc)
		{
			while(1)
			{
				int res = mysql_next_result(m_mysql);
				if(res == 0)
				{//有更多的数据集
					m_results = mysql_store_result(m_mysql);
					if( NULL == m_results )
					{
						break;
					}
					else
					{
						mysql_free_result(m_results);
						m_results = NULL;
					}
				}
				else
				{//没有数据集了，或者错误了
					break;
				}
			}
		}
		else
		{
			while(1)
			{
				m_results = mysql_store_result(m_mysql);
				if( NULL == m_results )
				{
					break;
				}
				else
				{
					mysql_free_result(m_results);
					m_results = NULL;
				}

				int res = mysql_next_result(m_mysql);
				if(res != 0)
				{//没有数据集了，或者错误了
					break;
				}
			}
		}

		m_record = NULL;
		m_field = NULL;
		m_rowNum = 0;
		m_fieldNum = 0;
		m_affectRows = 0;
		m_isFunc = false;

		m_fieldName.clear();
	}

private:

	/*
	 * 执行sql操作
	 * return :E_SQLResult
	 * param :要执行的语句
	 */
	int Operate(string &strSql, bool hasResults = false);

private:
	MYSQL* m_mysql;
	MYSQL_RES*  m_results;
	MYSQL_ROW  m_record;
	MYSQL_FIELD *m_field;

	unsigned int m_rowNum; // 记录数
	unsigned int m_fieldNum; //字段数
	int m_affectRows; //影响的行
	unsigned int m_key; //唯一标识这个连接的(只在被占用时才有)

	map<string, int> m_fieldName; //字段对应的序号
	bool m_isFunc;
};


/*
 * 数据库连接池单例类
 * 管理数据库连接
 *
 */
class CSqlConnPool
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:

	static CSqlConnPool * GetInstance()
	{
		if(NULL == m_instance)
		{
			m_instance = NEW CSqlConnPool();
		}

		return m_instance;
	}

	void DestroyInstance()
	{
		GUARD(CSimLock, obj, &m_idleMutex);
		map<unsigned int, CMysqlConn *>::iterator it = m_usedConn.begin();
		for(; it!=m_usedConn.end(); ++it)
		{
			delete it->second;
			it->second = NULL;
		}
		m_usedConn.clear();

		list<CMysqlConn *>::iterator itList = m_idleConn.begin();
		for(; itList!=m_idleConn.end(); ++itList)
		{
			delete (*itList);
			(*itList) = NULL;
		}
		m_idleConn.clear();
		obj.UnLock();

		if(m_instance)
		{
			delete m_instance;
		}

		m_instance = NULL;
	}

	/*
	 *初始化连接池
	 *return :初始化是否成功
	 *param :host主机  user用户名 pwd密码  db数据库名  port端口  size连接池初始大小   rate连接池大小增量
	 */
	int Init(string &host, string &user, string &pwd, string &db, int port, short int size, short int rate);

	/*
	 * 获取一个连接
	 * return :
	 * param :
	 */
	CMysqlConn *GetConnection();

	/*
	 * 释放一个连接
	 * return :
	 * param :
	 */
	void ReleaseConn(unsigned int onlyID);

private:
	CSqlConnPool();
	~CSqlConnPool();

	/*
	 * 增加一定数目的连接
	 * return :E_SQLResult
	 * param :
	 */
	int AddConnecton(short int count);

	/*
	 * 得到一个key值
	 * return :
	 * param ;
	 */
	unsigned int GetKey()
	{
		GUARD(CSimLock, obj, &m_keyMutex);
		if(m_key >= 1000000000)
		{
			m_key = 1;
		}

		unsigned int res = m_key++;

		return res;
	}

private:
	static CSqlConnPool * m_instance;

	short int m_poolSize; //连接的个数
	short int m_rate; //连接增量，用于连接池为空时，一次性增加的连接个数
	string m_host; //连接主机
	string m_user; //用户名
	string m_pwd; //密码
	string m_db; //数据库名
	int m_port; //端口

	map<unsigned int, CMysqlConn *> m_usedConn; //使用的连接
//	CSimLock m_usedMutex; //使用中的连接锁
	list<CMysqlConn *> m_idleConn; //空闲连接
	CSimLock m_idleMutex; //空闲的连接锁
	unsigned int m_key; //唯一id值
	CSimLock m_keyMutex; //空闲的连接锁
};
}

#endif /* SQLINTERFACE_SQLCONNPOOL_H_ */
