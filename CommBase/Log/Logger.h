/*
 * Logger.h
 *
 *  Created on: Jun 10, 2017
 *      Author: root
 */

#ifndef LOG_LOGGER_H_
#define LOG_LOGGER_H_

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "../Common.h"
#include "../Memory/MemAllocator.h"

using namespace std;

namespace CommBaseOut
{

enum E_LogRet
{
	eSuccess=0,
	eOpenFileError,
	eMakeDirError,
	eCreateThread,
	eNoHandle
};

/*
 *单种类型日志（信息，调试，警告等）
 *编译加宏  CLEARLOG ，作为测试用，单个日志国大，就会新建日志，新建之前会将以前的日志清掉
 *
 */
class CSingleLog
{
public:
	CSingleLog();
	~CSingleLog();

	/*
	 * 创建当前日志文件
	 * return: true成功    false失败
	 * param:
	 */
	bool CreateFile();

	/*
	 * 获取时间后缀
	 * return:
	 * param:
	 */
	string GetTimeSuffix();

	/*
	 * 初始化日志־
	 * return:E_LogRet类型
	 * param: name前缀名   len单个日志最大大小
	 */
	int Init(string &name, int len = 1024*1024*1024);

	/*
	 * 刷新缓冲区
	 * return:
	 * param:
	 */
	void Flush()
	{
		m_file.flush();
//		fflush(m_fileHandle);
	}

	/*
	 * 写入日志־
	 * return: bool
	 * param: content写入的内容
	 */
	bool writeLog(string &content);

private:
	int m_index;//当前日志序号
	int m_fileSize;//日志的大小
	int m_singleLogSize; //单个日志最大大小
	string m_prefixName;//日志的前缀名
//	FILE *m_fileHandle; //文件句柄
	ofstream m_file;
};

//写日志线程
void* WriteThread(void *p);

/*
 * 多线程日志单例类
 *
 *
 *
 */

class CLoggerMT
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
	enum E_LogType
	{
		E_Info=0,
		E_Debug,
		E_Warning,
		E_Error,
		E_Fatal,
		E_Base, //底层使用
		E_Max
	};

private:
	CLoggerMT();


public:

	 ~CLoggerMT();

	/*
	 * 初始化日志类
	 * return :E_LogRet类型
	 * param : 日志的开关    ,filePath 当前路径下面的日志的文件夹名     module当前模块名   len单个日志文件的最大容量
	 */
	int Init(int len, bool info, bool debug, bool warning, bool error, bool fatal, bool display, string &filePath, string &module, bool base = true);

	/*
	 * 获取日志类单件
	 * return : 日志对象
	 * param:
	 */
	static CLoggerMT * GetInstance()
	{
		if(NULL == m_instance)
		{
			m_instance = NEW CLoggerMT();
		}

		return m_instance;
	}

	/*
	 * 销毁日志单件
	 * return :
	 * param:
	 */
	void DestoryInstance()
	{
		m_threadEnd = true;

		if(m_event.WaitForSingleEvent())
		{
			cerr << "CLoggerMT::DestoryInstance DestroyThread error" << endl;
		}

		void *status = 0;

		pthread_join(m_id, &status);

		if(m_instance)
		{
			delete m_instance;
		}
	}

	/*
	 * 写入日志־
	 * return :
	 * param : format可格式化字串      ...为可变参数
	 */
	void LogInfo(const char *file, const char *function, int line, const char *format,...);
	void LogDebug(const char *file, const char *function, int line, const char *format,...);
	void LogWarning(const char *file, const char *function, int line, const char *format,...);
	void LogError(const char *file, const char *function, int line, const char *format,...);
	void LogFatal(const char *file, const char *function, int line, const char *format,...);
	void LogBase(const char *file, const char *function, int line, const char *format,...);

	/*
	 * 循环写日志־
	 * return :
	 * param :
	 */
	void run();

private:
	static CLoggerMT *m_instance;
	CSingleLog m_log[E_Max]; //所有类型的日志־
	bool m_enableLog[E_Max]; //是否开启各个类型的日志־
	bool m_enableDis; //是否打印到屏幕
	bool m_threadEnd; //线程结束标识
	vector<pair<int, string> > m_logList; //当前日志链
	CSimLock m_logMutex;
	pthread_t m_id; //线程id
	Event m_event;
};

}

#define LOG_INFO CLoggerMT::GetInstance()->LogInfo
#define LOG_DEBUG CLoggerMT::GetInstance()->LogDebug
#define LOG_WARNING CLoggerMT::GetInstance()->LogWarning
#define LOG_ERROR CLoggerMT::GetInstance()->LogError
#define LOG_FATAL CLoggerMT::GetInstance()->LogFatal
#define LOG_BASE CLoggerMT::GetInstance()->LogBase

#define FILEINFO __FILE__,__FUNCTION__,__LINE__

#endif /* LOG_LOGGER_H_ */
