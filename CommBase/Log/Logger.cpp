/*
 * Logger.cpp
 *
 *  Created on: Jun 10, 2017
 *      Author: root
 */

#include "Logger.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include "../util.h"

namespace CommBaseOut
{

CLoggerMT* CLoggerMT::m_instance = NULL;

CSingleLog::CSingleLog():m_index(1),m_fileSize(0),m_singleLogSize(0)//,m_fileHandle(NULL)
{

}

CSingleLog::~CSingleLog()
{
	m_file.close();
}

bool CSingleLog::CreateFile()
{
	std::string fileName = m_prefixName;
	char index[32] = {0};

	snprintf(index, 32, "_%d.log", m_index);
	fileName += index;

	if(m_file.is_open())
	{
		m_file.close();
		m_file.clear();
	}

/*#ifdef CLEARLOG
	char delIndex[32] = {0};
	string delName = m_prefixName;

	sprintf(delIndex, "_%d.log", m_index-1);
	delName += delIndex;

	FILE* fp = fopen(delName.c_str(), "w+");
	if(fp != NULL)
	{
		fclose(fp);
	}

#endif*/

	m_file.open(fileName.c_str());
	if(!m_file.is_open())
	{
		std::cerr << "CSingleLog::CreateFile open file error" << std::endl;

		return false;
	}

	m_fileSize = 0;
	m_index++;

	return true;
}

std::string CSingleLog::GetTimeSuffix()
{
	std::string timeStr;
	char timeCh[32] = {0};
	time_t tm;
	struct tm *st;

	time(&tm);
	st = localtime(&tm);

	snprintf(timeCh, 32, "%04d-%02d-%02d_%02d:%02d:%02d", (1900+st->tm_year), st->tm_mon+1, st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec);
	timeStr = timeCh;

	return timeStr;
}

int CSingleLog::Init(std::string &name, int len)
{
	m_prefixName = name + "_" + GetTimeSuffix();

	if(!CreateFile())
	{
		return eOpenFileError;
	}

	m_singleLogSize = len;

	return eSuccess;
}

bool CSingleLog::writeLog(std::string & content)
{
	if(!m_file.is_open())
	{
		std::cerr << "CSingleLog::writeLog handler error" << std::endl;
		return false;
	}

	m_file << content.c_str();
//	if(EOF == fputs(content.c_str(), m_fileHandle))
//	{
//		std::cerr << "CSingleLog::writeLog fputs error" << std::endl;
//		return false;
//	}

	m_fileSize += content.size();
	if(m_fileSize >= m_singleLogSize)
	{
		CreateFile();
	}

	return true;
}

void* WriteThread(void *p)
{
	CLoggerMT *log = (CLoggerMT *)p;

	log->run();

	return NULL;
}

CLoggerMT::CLoggerMT():m_enableDis(false),m_threadEnd(false),m_id(0)
{
	for(int i=E_Info; i<E_Max; ++i)
	{
		m_enableLog[i] = false;
	}
}

CLoggerMT::~CLoggerMT()
{
}

int CLoggerMT::Init(int len, bool info, bool debug, bool warning, bool error, bool fatal, bool display, std::string &filePath, std::string &module, bool base)
{
	char path[128] = {0};
	std::string sufix;

	getcwd(path, sizeof(path));
	sufix = path;
	sufix += "/";
	sufix += filePath;

	if(access(sufix.c_str(), F_OK))
	{
		if(mkdir(sufix.c_str(), 0777) < 0)
		{
			std::cerr << "CLoggerMT::Init mkdir error" << std::endl;

			return eMakeDirError;
		}
	}

	sufix += "/";
	sufix += module;
	sufix += "/";

	if(access(sufix.c_str(), F_OK))
	{
		if(mkdir(sufix.c_str(), 0777) < 0)
		{
			std::cerr << "CLoggerMT::Init mkdir error" << std::endl;

			return eMakeDirError;
		}
	}

	if(info)
	{
		m_enableLog[E_Info] = info;
		string infoSufix = sufix + "INFO";
		m_log[E_Info].Init(infoSufix, len);
	}

	if(debug)
	{
		m_enableLog[E_Debug] = debug;
		string debugSufix = sufix + "DEBUG";
		m_log[E_Debug].Init(debugSufix, len);
	}

	if(warning)
	{
		m_enableLog[E_Warning] = warning;
		string warningSufix = sufix + "WARNING";
		m_log[E_Warning].Init(warningSufix, len);
	}

	if(error)
	{
		m_enableLog[E_Error] = error;
		string errorSufix = sufix + "ERROR";
		m_log[E_Error].Init(errorSufix, len);
	}

	if(fatal)
	{
		m_enableLog[E_Fatal] = fatal;
		string fatalSufix = sufix + "FATAL";
		m_log[E_Fatal].Init(fatalSufix, len);
	}

	if(base)
	{
		m_enableLog[E_Base] = base;
		string fatalSufix = sufix + "BASE";
		m_log[E_Base].Init(fatalSufix, len);
	}

	if(display)
	{
		m_enableDis = display;
	}

	if(!m_threadEnd)
	{
		if(pthread_create(&m_id, NULL, WriteThread, this))
		{
			std::cerr << "CLoggerMT::Init Createthread error" << std::endl;

			return eCreateThread;
		}
	}

	return eSuccess;
}


void  CLoggerMT::LogInfo(const char *file, const char *function, int line, const char *format,...)
{
	if (!m_enableLog[E_Info])
		return;

	const char *log_type = "LOGINFO";
	char log_data[4096] = {0};

	size_t off = 0;

	off += sprintf(log_data + off, "\n%s\n",log_type);
	std::string timeSufix = m_log[E_Info].GetTimeSuffix();
	off += sprintf(log_data + off, "%s FILE:%s FUNC:%s LINE:%d\n 	  Message:",timeSufix.c_str(),file, function, line);

	va_list   args;
	va_start(args,format);
	vsnprintf(log_data +off, 4096 - off, format, args);
	va_end(args);

	string tmpstr = log_data;

	if(m_enableDis)
	{
		printf("%s", tmpstr.c_str());
	}

	GUARD(CSimLock, obj, &m_logMutex);
	m_logList.push_back(make_pair(E_Info, tmpstr));
}

void  CLoggerMT::LogDebug(const char *file, const char *function, int line, const char *format,...)
{
	if (!m_enableLog[E_Debug])
		return;

	const char *log_type = "LOGDEBUG";
	char log_data[4096] = {0};

	size_t off = 0;

	off += sprintf(log_data + off, "\n%s\n",log_type);
	std::string timeSufix = m_log[E_Debug].GetTimeSuffix();
	off += sprintf(log_data + off, "%s FILE:%s FUNC:%s LINE:%d\n 	  Message:",timeSufix.c_str(),file, function, line);

	va_list   args;
	va_start(args,format);
	vsnprintf(log_data +off, 4096 - off, format, args);
	va_end(args);

	string tmpstr = log_data;

	if(m_enableDis)
	{
		printf("%s", tmpstr.c_str());
	}

	GUARD(CSimLock, obj, &m_logMutex);
	m_logList.push_back(make_pair(E_Debug, tmpstr));
}

void  CLoggerMT::LogWarning(const char *file, const char *function, int line, const char *format,...)
{
	if (!m_enableLog[E_Warning])
		return;

	const char *log_type = "LOGWARNING";
	char log_data[4096] = {0};

	size_t off = 0;

	off += sprintf(log_data + off, "\n%s\n",log_type);
	std::string timeSufix = m_log[E_Warning].GetTimeSuffix();
	off += sprintf(log_data + off, "%s FILE:%s FUNC:%s LINE:%d\n 	  Message:",timeSufix.c_str(),file, function, line);

	va_list   args;
	va_start(args,format);
	vsnprintf(log_data +off, 4096 - off, format, args);
	va_end(args);

	string tmpstr = log_data;

	if(m_enableDis)
	{
		printf("%s", tmpstr.c_str());
	}

	GUARD(CSimLock, obj, &m_logMutex);
	m_logList.push_back(make_pair(E_Warning, tmpstr));
}

void  CLoggerMT::LogError(const char *file, const char *function, int line, const char *format,...)
{
	if (!m_enableLog[E_Error])
		return;

	const char *log_type = "LOGERROR";
	char log_data[4096] = {0};

	size_t off = 0;

	off += sprintf(log_data + off, "\n%s\n",log_type);
	std::string timeSufix = m_log[E_Error].GetTimeSuffix();
	off += sprintf(log_data + off, "%s FILE:%s FUNC:%s LINE:%d\n 	  Message:",timeSufix.c_str(),file, function, line);

	va_list   args;
	va_start(args,format);
	vsnprintf(log_data +off, 4096 - off, format, args);
	va_end(args);

	string tmpstr = log_data;

	if(m_enableDis)
	{
		printf("%s", tmpstr.c_str());
	}

	GUARD(CSimLock, obj, &m_logMutex);
	m_logList.push_back(make_pair(E_Error, tmpstr));
}

void  CLoggerMT::LogFatal(const char *file, const char *function, int line, const char *format,...)
{
	if (!m_enableLog[E_Fatal])
		return;

	const char *log_type = "LOGFATAL";
	char log_data[4096] = {0};

	size_t off = 0;

	off += sprintf(log_data + off, "\n%s\n",log_type);
	std::string timeSufix = m_log[E_Fatal].GetTimeSuffix();
	off += sprintf(log_data + off, "%s FILE:%s FUNC:%s LINE:%d\n 	  Message:",timeSufix.c_str(),file, function, line);

	va_list   args;
	va_start(args,format);
	vsnprintf(log_data +off, 4096 - off, format, args);
	va_end(args);

	string tmpstr = log_data;

	if(m_enableDis)
	{
		printf("%s", tmpstr.c_str());
	}

	GUARD(CSimLock, obj, &m_logMutex);
	m_logList.push_back(make_pair(E_Fatal, tmpstr));
}

void  CLoggerMT::LogBase(const char *file, const char *function, int line, const char *format,...)
{
	if (!m_enableLog[E_Base])
		return;

	const char *log_type = "LOGBASE";
	char log_data[4096] = {0};

	size_t off = 0;

	off += sprintf(log_data + off, "\n%s\n",log_type);
	std::string timeSufix = m_log[E_Base].GetTimeSuffix();
	off += sprintf(log_data + off, "%s FILE:%s FUNC:%s LINE:%d\n 	  Message:",timeSufix.c_str(),file, function, line);

	va_list   args;
	va_start(args,format);
	vsnprintf(log_data +off, 4096 - off, format, args);
	va_end(args);

	string tmpstr = log_data;

	if(m_enableDis)
	{
		printf("%s", tmpstr.c_str());
	}

	GUARD(CSimLock, obj, &m_logMutex);
	m_logList.push_back(make_pair(E_Base, tmpstr));
}

void CLoggerMT::run()
{
	while(!m_threadEnd)
	{
		try
		{
			GUARD(CSimLock, obj, &m_logMutex);
			std::vector<std::pair<int, std::string> > tList;
			tList.insert(tList.end(), m_logList.begin(), m_logList.end());
//			m_logList.clear();
			std::vector<std::pair<int, std::string> >().swap(m_logList);
			obj.UnLock();

			if(tList.size() <= 0)
			{
				CUtil::MSleep(100);
				continue;
			}

			int pos = 0;
			if(tList.size() > 10000)
			{
				std::cerr << "CLoggerMT::run logbuff beyond MaxNum" << std::endl;
				pos = tList.size() - 10000;
			}

			bool enableLog[E_Max];
			for(size_t i=pos; i<tList.size(); ++i)
			{
				if(!m_log[tList[i].first].writeLog(tList[i].second))
				{
					std::cerr << "CLoggerMT::run writeLog error" << std::endl;
				}

				enableLog[tList[i].first] = true;
			}

			for(size_t j=0; j<E_Max; ++j)
			{
				if(enableLog[j])
					m_log[j].Flush();
			}

			tList.clear();
		}
		catch(exception &e)
		{
			std::cerr << "CLoggerMT::run writeLog fatal error and error msg :" << e.what() << std::endl;

			continue;
		}
		catch(...)
		{
			std::cerr << "CLoggerMT::run writeLog fatal error" << std::endl;

			continue;
		}

		CUtil::MSleep(20);
	}

	m_event.SetEvent();
}

}
