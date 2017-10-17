/*
 * RedisConnPool.h
 *
 *  Created on: Oct 10, 2017
 *      Author: root
 */

#ifndef REDIS_REDISCONNPOOL_H_
#define REDIS_REDISCONNPOOL_H_

#include <hiredis/hiredis.h>
#include <string>
#include "../Common.h"
#include <list>
#include <map>
#include "../define.h"
#include "../Memory/MemAllocator.h"

using namespace std;

namespace CommBaseOut
{

	enum E_RedisResult
	{
		eRedisSuccess=0, //成功
//		eSeleNULL, //查询结果为0个数据
		eRedisHandleNULL, //数据库连接句柄为空
//		eExeError, //执行错误
//		eResError, //返回结果错误
//		eBefExeError, //上一次执行失败
//		eCreateConError, //新建连接错误
		eConnRedisError, //连接Redis失败
	};

	/*
	 * Redis抛出的异常
	 */
	class RedisException : public std::exception
	{
	public :
		RedisException(string &dec):m_what(dec){}
		virtual ~RedisException() throw()
		{}

		virtual const char* what() const throw()
		{
			return m_what.c_str();
		}

	private:
		string m_what;
	};

	/*
	 *单个连接，封装Redis操作
	 */
	class CRedisConn
	#ifdef USE_MEMORY_POOL
	: public MemoryBase
	#endif
	{
	public:
		CRedisConn();
		~CRedisConn();

		/*
		 * 连接数据库
		 * return : 主机名或者ip  端口
		 * param :
		 */
		bool Connect(string &host, int port = 0);

		string Hget(const char* key,const  char* hkey);

		int Hset(const char* key, const char* hkey, const char* value);

		int Hset(const char* key,const char* hkey,const char* hvalue, size_t hvaluelen);

		int Del(const char* key);

		int ExistsKey(const char* id);

		bool ExecuteCommandV(const char *format, va_list ap);

		bool ExecuteCommandArgv(int argc, const char **argv, const size_t *argvlen);

		bool ExecuteCommandInPipelineV(const char *format, va_list ap);

		bool GetReplyInPipeline();

	public:
		bool Status() const;

		bool Error() const;

		bool Integer() const;

		bool Nil() const;

		bool String() const;

		bool Array() const;

		const char* GetStatus(const char* szNullValue="");

		const char* GetError(const char* szNullValue="");

		int64 GetInteger(int64 nNullValue=0);

		const char* GetString(const char* szNullValue="");

		const redisReply* GetArray();

		bool GetArryToList(vector<string>& valueList);

		bool GetArryToMap(map<string,string>& valueMap);
	public:
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

		void Clear()
		{
			if(m_reply)
			{
				freeReplyObject(m_reply);
				m_reply = NULL;
			}
		}

	private:
		redisContext* m_connect;
		redisReply* m_reply;
		unsigned int m_key;
		int m_pending_reply_count;
	};


	/*
	 * 数据库连接池单例类
	 * 管理数据库连接
	 *
	 */
	class CRedisConnPool
	#ifdef USE_MEMORY_POOL
	: public MemoryBase
	#endif
	{
	public:
		static CRedisConnPool* GetInstance()
		{
			if(NULL == m_instance)
			{
				m_instance = NEW CRedisConnPool();
			}

			return m_instance;
		}

		void DestroyInstance()
		{
			GUARD(CSimLock, obj, &m_idleMutex);
			map<unsigned int, CRedisConn *>::iterator it = m_usedConn.begin();
			for(; it!=m_usedConn.end(); ++it)
			{
				delete it->second;
				it->second = NULL;
			}
			m_usedConn.clear();

			list<CRedisConn *>::iterator itList = m_idleConn.begin();
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
		 *param :host主机 pwd密码  port端口  size连接池初始大小   rate连接池大小增量
		 */
		int Init(string &host, string &pwd, int port, short int size, short int rate);

		/*
		 * 获取一个连接
		 * return :
		 * param :
		 */
		CRedisConn *GetConnection();

		/*
		 * 释放一个连接
		 * return :
		 * param :
		 */
		void ReleaseConn(unsigned int onlyID);

	private:
		CRedisConnPool();
		~CRedisConnPool();

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
		static CRedisConnPool * m_instance;

		short int m_poolSize; //连接的个数
		short int m_rate; //连接增量，用于连接池为空时，一次性增加的连接个数
		string m_host; //连接主机
		string m_pwd; //密码
		int m_port; //端口

		map<unsigned int, CRedisConn *> m_usedConn; //使用的连接
	//	CSimLock m_usedMutex; //使用中的连接锁
		list<CRedisConn *> m_idleConn; //空闲连接
		CSimLock m_idleMutex; //空闲的连接锁
		unsigned int m_key; //唯一id值
		CSimLock m_keyMutex; //空闲的连接锁
	};

}

#endif /* REDIS_REDISCONNPOOL_H_ */
