/*
 * CRedisInterface.cpp
 *
 *  Created on: Oct 11, 2017
 *      Author: root
 */

#include "CRedisInterface.h"
#include "../Log/Logger.h"

namespace CommBaseOut
{
	CRedisInterface::CRedisInterface():m_con(NULL)
	{
		m_con = CRedisConnPool::GetInstance()->GetConnection();
	}

	CRedisInterface::~CRedisInterface()
	{
		ReleaseConnection();
	}

	void CRedisInterface::ReleaseConnection()
	{
		if(NULL == m_con)
		{
			char dec[128] = {0};
			string what;

			snprintf(dec, 128, "[CRedisInterface::ReleaseConnection] Releasing NULL Connection");
			what = dec;

			LOG_BASE(FILEINFO, "release connection failed,error msg[%s]", dec);

			throw RedisException(what);
		}

		CRedisConnPool::GetInstance()->ReleaseConn(m_con->GetKey());
	}

	bool CRedisInterface::ExecuteCommand(const char *format, ...)
	{
	    va_list ap;
	    va_start(ap, format);
	    bool ret = m_con->ExecuteCommandV(format, ap);
	    va_end(ap);
	    return ret;
	}

	bool CRedisInterface::ExecuteCommandArgv(int argc, const char **argv, const size_t *argvlen)
	{
		return m_con->ExecuteCommandArgv(argc,argv,argvlen);
	}

	bool CRedisInterface::ExecuteCommandInPipeline(const char *format, ...)
	{
	    va_list ap;
	    va_start(ap, format);
	    bool ret = m_con->ExecuteCommandInPipelineV(format, ap);
	    va_end(ap);
	    return ret;
	}

	bool CRedisInterface::GetReplyInPipeline()
	{
		return m_con->GetReplyInPipeline();
	}
}
