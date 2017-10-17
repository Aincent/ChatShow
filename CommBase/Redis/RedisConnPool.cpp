/*
 * RedisConnPool.cpp
 *
 *  Created on: Oct 10, 2017
 *      Author: root
 */

#include "RedisConnPool.h"
#include "../Log/Logger.h"

namespace CommBaseOut
{
	CRedisConnPool *CRedisConnPool::m_instance = NULL;

	CRedisConn::CRedisConn():m_connect(NULL),m_reply(NULL),m_key(0),m_pending_reply_count(0)
	{

	}

	CRedisConn::~CRedisConn()
	{
		if(this->m_connect)
		{
			redisFree(this->m_connect);
		}

		Clear();
	}

	bool CRedisConn::Connect(string &host, int port)
	{
		this->m_connect = redisConnect(host.c_str(),port);
		if(m_connect == NULL || m_connect->err)
		{
			if(m_connect)
			{
				char dec[512] = {0};
				string what;

				snprintf(dec, 512, "[CRedisConn::Connect] : message = [Create connection failed], errno = [%s]",m_connect->errstr);
				what = dec;

				LOG_BASE(FILEINFO, "AddConnecton failed,error msg[%s]", dec);

				throw RedisException(what);
			}
			else
			{
				char dec[512] = {0};
				string what;

				snprintf(dec, 512, "[CRedisConn::Connect] : message = [can not allocate redis context");
				what = dec;

				LOG_BASE(FILEINFO, "AddConnecton failed,error msg[%s]", dec);

				throw RedisException(what);
			}

			return false;
		}

		return true;
	}


	string CRedisConn::Hget(const char* key,const  char* hkey)
	{
		const char* argv[3];
		size_t argvlen[3];
		argv[0] = "HGET";
		argvlen[0] = 4;
		argv[1] = key;
		argvlen[1] = strlen(key);
		argv[2] = hkey;
		argvlen[2] = strlen(hkey);
		redisReply* reply =(redisReply*) redisCommandArgv(this->m_connect, 3, argv, argvlen);
		string value;
		if(reply->type != REDIS_REPLY_NIL){
			value = string(reply->str,reply->str + reply->len);
		}
		freeReplyObject(reply);
		return value;
	}

	int CRedisConn::Hset(const char* key, const char* hkey, const char* value)
	{
		redisReply* reply =(redisReply*) redisCommand(this->m_connect, "HSET %s %s %s",key,hkey, value);
		freeReplyObject(reply);
		return 1;
	}

	int CRedisConn::Hset(const char* key,const char* hkey,const char* hvalue, size_t hvaluelen)
	{
        const char* argv[4];
        size_t argvlen[4];
        argv[0] = "HSET";
        argvlen[0] = 4;
        argv[1] = key;
        argvlen[1] = strlen(key);
        argv[2] = hkey;
        argvlen[2] = strlen(hkey);
        argv[3] = hvalue;
        argvlen[3] = hvaluelen;
        redisReply * reply =(redisReply*) redisCommandArgv(this->m_connect, 4, argv, argvlen);
        freeReplyObject(reply);
        return 1;
	}

	int CRedisConn::Del(const char* key)
	{
        int res = 0;
        redisReply* reply = (redisReply*)redisCommand(this->m_connect, "DEL %s", key);
        if(reply->type == REDIS_REPLY_INTEGER){
            if(reply->integer == 1L)
                res = 1;
        }
        freeReplyObject(reply);
        return res;
	}

	int CRedisConn::ExistsKey(const char* key)
	{
        redisReply * reply = (redisReply*)redisCommand(this->m_connect,"exists %s",key);
        int res = 0;
        if(reply->type == REDIS_REPLY_INTEGER){
            if(reply->integer == 1L)
                res  = 1;
        }
        freeReplyObject(reply);
        return res;
	}

	bool CRedisConn::ExecuteCommandV(const char *format, va_list ap)
	{
		Clear();

	    this->m_reply = (redisReply*)redisvCommand(this->m_connect, format, ap);
	    if (this->m_reply == NULL)
	    {
	        return false;
	    }

	    return true;
	}

	bool CRedisConn::ExecuteCommandArgv(int argc, const char **argv, const size_t *argvlen)
	{
		Clear();

	    this->m_reply = (redisReply*)redisCommandArgv(this->m_connect, argc, argv,argvlen);
	    if (this->m_reply == NULL)
	    {
	        return false;
	    }

	    return true;
	}


	bool CRedisConn::ExecuteCommandInPipelineV(const char *format, va_list ap)
	{
		int r = redisvAppendCommand(this->m_connect, format, ap);
		if (r == REDIS_ERR)
		{
			return false;
		}

		m_pending_reply_count++;
		return true;
	}

	bool CRedisConn::GetReplyInPipeline()
	{
		if(m_pending_reply_count <= 0)
			return false;

		m_pending_reply_count--;
		Clear();
	    if (redisGetReply(this->m_connect, (void**)&this->m_reply) != REDIS_OK)
	    {
	        return false;
	    }

	    return true;
	}

	bool CRedisConn::Status() const
	{
		return this->m_reply != NULL && this->m_reply->type == REDIS_REPLY_STATUS;
	}

	bool CRedisConn::Error() const
	{
		return this->m_reply != NULL || this->m_reply->type == REDIS_REPLY_ERROR;
	}

	bool CRedisConn::Integer() const
	{
		return this->m_reply != NULL && this->m_reply->type == REDIS_REPLY_INTEGER;
	}

	bool CRedisConn::Nil() const
	{
		return this->m_reply != NULL && this->m_reply->type == REDIS_REPLY_NIL;
	}

	bool CRedisConn::String() const
	{
		return this->m_reply != NULL && this->m_reply->type == REDIS_REPLY_STRING;
	}

	bool CRedisConn::Array() const
	{
		return this->m_reply != NULL && this->m_reply->type == REDIS_REPLY_ARRAY;
	}

	const char* CRedisConn::GetStatus(const char* szNullValue)
	{
		if(Status())
		{
			string value = string(this->m_reply->str,this->m_reply->str + this->m_reply->len);
			return value.c_str();
		}

		return szNullValue;
	}

	const char* CRedisConn::GetError(const char* szNullValue)
	{
		if(Error())
		{
			string value = string(this->m_reply->str,this->m_reply->str + this->m_reply->len);
			return value.c_str();
		}

		return szNullValue;
	}

	int64 CRedisConn::GetInteger(int64 nNullValue)
	{
		if(Integer())
		{
			return this->m_reply->integer;
		}

		return nNullValue;
	}

	const char* CRedisConn::GetString(const char* szNullValue)
	{
		if(String())
		{
			string value = string(this->m_reply->str,this->m_reply->str + this->m_reply->len);
			return value.c_str();
		}

		return szNullValue;
	}

	const redisReply* CRedisConn::GetArray()
	{
		if(Array())
		{
			return this->m_reply;
		}

		return NULL;
	}

	bool CRedisConn::GetArryToList(vector<string>& valueList)
	{
		if(Array())
		{
			std::size_t num = this->m_reply->elements;

			for ( std::size_t i = 0 ; i < num ; i++ )
			{
				valueList.push_back( this->m_reply->element[i]->str );
			}

			return true;
		}

		return false;
	}

	bool CRedisConn::GetArryToMap(map<string,string>& valueMap)
	{
		if(Array())
		{
			std::size_t num = this->m_reply->elements;

			for ( std::size_t i = 0 ; i < num ; i += 2 )
			{
				valueMap.insert(std::pair<std::string , std::string>(this->m_reply->element[i]->str,this->m_reply->element[i + 1]->str));
			}

			return true;
		}

		return false;
	}

	CRedisConnPool::CRedisConnPool():m_poolSize(0),m_rate(0),m_port(0),m_key(1)
	{

	}

	CRedisConnPool::~CRedisConnPool()
	{

	}

	int CRedisConnPool::AddConnecton(short int count)
	{
		for(int i = 0; i < count; i++)
		{
			CRedisConn *conn = NEW CRedisConn();
			if(!conn->Connect(m_host,m_port))
			{
				delete conn;
				conn = NULL;

				char dec[512] = {0};
				string what;

				snprintf(dec, 512, "[CRedisConnPool::AddConnecton] : message = [new connection init failed], errno = [%d]",eRedisHandleNULL);
				what = dec;

				LOG_BASE(FILEINFO, "AddConnecton failed,error msg[%s]", dec);

				throw RedisException(what);
			}

			unsigned int key = GetKey();
			conn->SetKey(key);

			GUARD(CSimLock, objIdle, &m_idleMutex);
			m_idleConn.push_back(conn);

			m_poolSize++;
			objIdle.UnLock();
		}

		return eRedisSuccess;
	}

	int CRedisConnPool::Init(string &host, string &pwd, int port, short int size, short int rate)
	{
		m_host = host;
		m_pwd = pwd;
		m_port = port;
		m_rate = rate;

		return AddConnecton(size);
	}

	CRedisConn *CRedisConnPool::GetConnection()
	{
		GUARD(CSimLock, objIdle, &m_idleMutex);
		int size = m_idleConn.size();
		if(size == 0)
		{
			char dec[128] = {0};
			string what;

			snprintf(dec, 128, "[CRedisConnPool::GetConnection] idle connection is zero");
			what = dec;

			LOG_BASE(FILEINFO, "GetConnection failed,error msg[%s]", dec);

			throw RedisException(what);
		}

		CRedisConn *conn = m_idleConn.front();
		m_idleConn.pop_front();

		//	GUARD(CSimLock, obj, &m_usedMutex);
		m_usedConn[conn->GetKey()] = conn;
		objIdle.UnLock();

		if((size - 1) < m_rate)
		{
			AddConnecton(m_rate);
		}

		return conn;
	}

	void CRedisConnPool::ReleaseConn(unsigned int onlyID)
	{
		GUARD(CSimLock, obj, &m_idleMutex);
		map<unsigned int, CRedisConn *>::iterator it = m_usedConn.find(onlyID);
		if(it == m_usedConn.end())
		{
			char dec[128] = {0};
			string what;

			snprintf(dec, 128, "[CRedisConnPool::ReleaseConn] Failed when Releasing Connection[ID=%u] to pool",onlyID);
			what = dec;

			LOG_BASE(FILEINFO, "ReleaseConn failed,error msg[%s]", dec);

			throw RedisException(what);
		}

		CRedisConn * conn = it->second;
		m_usedConn.erase(it);

	//	con->SetKey(0);
		m_idleConn.push_back(conn);
	}
}
