/*
 * ThreadPool.h
 *
 *  Created on: Jun 10, 2017
 *      Author: root
 */

#ifndef THREAD_THREADPOOL_H_
#define THREAD_THREADPOOL_H_

#include "../Common.h"
#include "../define.h"
#include "../Memory/MemAllocator.h"

namespace CommBaseOut
{

/*
 * 单例类
 * 用于执行有限次任务
 */
class CThreadPool
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:
	~CThreadPool();

	static CThreadPool *GetInstance()
	{
		if(m_instance == 0)
		{
			m_instance = NEW CThreadPool();
		}

		return m_instance;
	}

	/*
	 *初始化线程池线程
	 *return :
	 *param :dwThreadNum 线程池开启线程数目
	 */
  void InitThread(DWORD dwThreadNum);

  /*
   * 增加一个任务
 * return :
 * param :void *(*pFunThread)(void *) 要执行的线程函数       arg 要执行函数的参数
   */
	void AddTask(void *(*pFunThread)(void *), void *arg);

	void DestroyInstance();

private:
	CThreadPool();

	enum enThreadState
    {
        THREAD_IDLE,
        THREAD_RUNNING
    };

	struct tagThreadInfo
	{
		void *(*pFun)(void *);
		void *arg;
		tagThreadInfo()
        {
            pFun = 0;
            arg = 0;
        }
    };

	static void *ThreadWork(void *arg);

	static CThreadPool *m_instance; //全局唯一实例
	DWORD m_dwThreadNum; //线程数
	bool m_bExitThread; //是否退出线程池
	pthread_t *m_pID;
	std::queue<struct tagThreadInfo> m_queueTask; //任务队列
	CSimLock m_mutexTask;
	static const DWORD THREAD_WAIT = 100; //线程轮询时间
	Event  m_event;
};

}

#endif /* THREAD_THREADPOOL_H_ */
