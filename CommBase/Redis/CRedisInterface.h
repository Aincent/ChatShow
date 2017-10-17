/*
 * CRedisInterface.h
 *
 *  Created on: Oct 11, 2017
 *      Author: root
 */

#ifndef REDIS_CREDISINTERFACE_H_
#define REDIS_CREDISINTERFACE_H_

#include <string>
#include <stdarg.h>
#include "RedisConnPool.h"

using namespace std;

namespace CommBaseOut
{
	class CRedisInterface
	{
	public:
		CRedisInterface();
		~CRedisInterface();

		/*
		 * 释放连接
		 * return :
		 * param :
		 */

		void ReleaseConnection();

		bool ExecuteCommand(const char *format, ...);

		bool ExecuteCommandArgv(int argc, const char **argv, const size_t *argvlen);

		bool ExecuteCommandInPipeline(const char *format, ...);

		bool GetReplyInPipeline();

	public:

		const char* GetStatus()
		{
			return m_con->GetStatus();
		}

		const char* GetError()
		{
			return m_con->GetError();
		}

		int64 GetInteger()
		{
			return m_con->GetInteger();
		}

		const char* GetString()
		{
			return m_con->GetString();
		}

		const redisReply* GetArray()
		{
			return m_con->GetArray();
		}

		bool GetArryToList(vector<string>& valueList)
		{
			return m_con->GetArryToList(valueList);
		}

		bool GetArryToMap(map<string,string>& valueMap)
		{
			return m_con->GetArryToMap(valueMap);
		}

	private:
		CRedisConn * m_con;
	};
}

#endif /* REDIS_CREDISINTERFACE_H_ */
