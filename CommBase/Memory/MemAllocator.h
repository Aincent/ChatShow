/*
 * MemAllocator.h
 *
 *  Created on: Jun 10, 2017
 *      Author: root
 */

#ifndef MEMORY_MEMALLOCATOR_H_
#define MEMORY_MEMALLOCATOR_H_

#include <stdio.h>
#include "AllocatorBlock.h"
#include "../Thread/Task.h"
#include <map>
#include "../Common.h"
#include <string.h>

using namespace std;

namespace CommBaseOut
{

struct MemoryLogInfo
{
	char file[256];
	int line;
	char function[64];
	int size;

	MemoryLogInfo():line(0),size(0)
	{
		memset(file, 0, sizeof(file));
		memset(function, 0, sizeof(function));
	}
};

class MemoryLeaks : public Task
{
public:
	MemoryLeaks();
	virtual ~MemoryLeaks();

	static  MemoryLeaks*  GetInstance(void)
	{
		if(m_instance == NULL)
		{
			if(m_instance == NULL)
			{
				m_instance = new MemoryLeaks;
			}
		}

		return m_instance;
	}

	void DestroyInstance()
	{
		if(m_instance)
		{
			delete m_instance;
			m_instance = 0;
		}
	}

	/*
	 *  初始化内存检测
	 * param : time 多久打印一次日志， 以秒为单位
	 * ｒｅｔｕｒｎ　:
	 */
	void Init(int time);

	void AddLog(void * p, size_t size, const char *file, int line, const char *function);
	void DeleteLog(void *p);
	void WriteLog();
	void CreateFile();


private:

	virtual int svr();

private:

	static MemoryLeaks *m_instance;

	map<void *, MemoryLogInfo> m_memoryLog;
	CSimLock m_lock;

	int m_time;
	int m_fileSize;//日志的大小
	FILE *m_fileHandle; //文件句柄
};


class MemAllocator
{
public:

	~MemAllocator()
	{

	}

private:

	MemAllocator():Allocator16_(16),Allocator32_(32),Allocator64_(64),Allocator128_(128),Allocator256_(256),Allocator512_(512),Allocator1024_(1024),Allocator2048_(2048),
		Allocator4096_(4096),Allocator8192_(8192),Allocator16384_(16384),Allocator32768_(32768),Allocator65536_(65536)
	{

	}

public:

	void* realloc(void* p,size_t  rbytes);

	void*  malloc(size_t nbytes, const char *file, int line, const char *function);

	void* move_void_ptr(void* ptr,int i);

	void free(void* p);

	void free(void* p,size_t nbytes);

	static  MemAllocator*  GetInstance(void)
	{
		if( !Allocator_ )
		{
			Allocator_ = new MemAllocator();
		}

		return Allocator_;
	}

	void DestroyInstance()
	{
		if(Allocator_)
		{
			delete Allocator_;
			Allocator_ = 0;
		}
	}

	size_t  get_total_allocator_size(void)
	{
		return Allocator16_.get_allocator_size() + Allocator32_.get_allocator_size() + Allocator64_.get_allocator_size() +Allocator128_.get_allocator_size() + Allocator256_.get_allocator_size()
			+ Allocator512_.get_allocator_size() +  Allocator1024_.get_allocator_size() + Allocator2048_.get_allocator_size() +  Allocator4096_.get_allocator_size()
			+ Allocator8192_.get_allocator_size() + Allocator16384_.get_allocator_size() + Allocator32768_.get_allocator_size() + Allocator65536_.get_allocator_size();
	}

	void WriteLog()
	{

	}

private:

	AllocatorBlock Allocator16_;
	AllocatorBlock Allocator32_;
	AllocatorBlock Allocator64_;
	AllocatorBlock Allocator128_;
	AllocatorBlock Allocator256_;
	AllocatorBlock Allocator512_;
	AllocatorBlock Allocator1024_;
	AllocatorBlock Allocator2048_;
	AllocatorBlock Allocator4096_;
	AllocatorBlock Allocator8192_;
	AllocatorBlock Allocator16384_;
	AllocatorBlock Allocator32768_;
	AllocatorBlock Allocator65536_;

	static MemAllocator *Allocator_;
};

class MemoryBase
{
public:
	virtual ~MemoryBase(){}

	void * operator new (size_t size, const char *file, int line, const char *function);
	void * operator new[] (size_t size, const char *file, int line, const char *function);
	void operator delete (void * p, const char *file, int line, const char *function);
	void operator delete[] (void * p, const char *file, int line, const char *function);
	void operator delete[] (void * p);
	void operator delete (void * p);
};

}

enum EMemoryType
{
	eMemorySingle=0,
	eMemoryArray
};

#ifndef NEW

#ifdef USE_MEMORY_POOL

#define NEW new(__FILE__, __LINE__, __FUNCTION__)
#define NEW_SINGLE(o) ((o *)CommBaseOut::MemAllocator::GetInstance()->malloc(sizeof(o), __FILE__, __LINE__, __FUNCTION__))
#define NEW_BASE(o, c) ((o *)CommBaseOut::MemAllocator::GetInstance()->malloc(sizeof(o) * c, __FILE__, __LINE__, __FUNCTION__))
#define DELETE_BASE(p, t) CommBaseOut::MemAllocator::GetInstance()->free(p)


#else

#define NEW new
#define NEW_SINGLE(o) new o
#define NEW_BASE(o, c) ( new o[c] )
#define DELETE_BASE(p, t) ( (t > 0) ? (delete [] p) : (delete p) )

#endif

#endif

#ifndef DELETE

#define DELETE delete

#endif

#endif /* MEMORY_MEMALLOCATOR_H_ */
