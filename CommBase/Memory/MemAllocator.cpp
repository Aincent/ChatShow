/*
 * MemAllocator.cpp
 *
 *  Created on: Jun 10, 2017
 *      Author: root
 */

#include "MemAllocator.h"
#include "../Log/Logger.h"
#include <string.h>
#include "../util.h"

namespace CommBaseOut
{

MemoryLeaks *MemoryLeaks::m_instance = 0;

MemoryLeaks::MemoryLeaks():m_time(1),m_fileSize(0),m_fileHandle(0)
{

}

MemoryLeaks::~MemoryLeaks()
{
}

void MemoryLeaks::Init(int time)
{
	m_time = time;
}

void MemoryLeaks::CreateFile()
{
	std::string fileName = "MemoryLeaks_";
	std::string timeStr;
	char timeCh[32] = {0};
	time_t tm;
	struct tm *st;

	time(&tm);
	st = localtime(&tm);

	snprintf(timeCh, 32, "%04d-%02d-%02d_%02d:%02d:%02d", (1900+st->tm_year), st->tm_mon+1, st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec);
	timeStr = timeCh;
	fileName = fileName + timeStr;

	if(m_fileHandle)
	{
		fclose(m_fileHandle);
		m_fileHandle = NULL;
	}

	m_fileHandle = fopen(fileName.c_str(), "w");
	if(NULL == m_fileHandle)
	{
		std::cerr << "CSingleLog::CreateFile open file error" << std::endl;

		return ;
	}

	m_fileSize = 0;
}

void MemoryLeaks::AddLog(void * p, size_t size, const char *file, int line, const char *function)
{
//	MemoryLogInfo info;
//
//	info.line = line;
//	info.size = size;
//	CUtil::SafeMemmove(info.file, 255, file, strlen(file));
//	CUtil::SafeMemmove(info.file, 255, function, strlen(function));
//
//	GUARD(CommBaseOut::CSimLock, obj, &m_lock);
//	m_memoryLog[p] = info;
//	obj.UnLock();
}

void MemoryLeaks::DeleteLog(void *p)
{
//	GUARD(CommBaseOut::CSimLock, obj, &m_lock);
//	map<void *, MemoryLogInfo>::iterator it = m_memoryLog.find(p);
//	if(it != m_memoryLog.end())
//	{
//		m_memoryLog.erase(it);
//	}
//	else
//	{
////		printf("\n\n+++++++++++++++++++++++++++++++++++memory[ 0x%x ] is not in pool+++++++++++++++++++++++++\n\n", p);
//	}
}

void MemoryLeaks::WriteLog()
{
	if(m_fileHandle == NULL)
	{
		std::cerr << "MemoryLeaks::writeLog handler error" << std::endl;
		return ;
	}

	char log[512] = {0};
	int len = 0;

	snprintf(log, 511, "\n\n-----------------------------------memory leaks-------------------------------\n\n");
	snprintf(log, 511 - strlen(log), "                            memory total size[%zu]                           \n", CommBaseOut::MemAllocator::GetInstance()->get_total_allocator_size());

	GUARD(CommBaseOut::CSimLock, obj, &m_lock);
	snprintf(log, 511 - strlen(log), "                            memory used chunck count[%zu]                           \n\n", m_memoryLog.size());

	if(EOF == fputs(log, m_fileHandle))
	{
		std::cerr << "MemoryLeaks::writeLog fputs error" << std::endl;
		return ;
	}

	len += strlen(log);
	map<void *, MemoryLogInfo>::iterator it = m_memoryLog.begin();
	for(; it!=m_memoryLog.end(); ++it)
	{
		memset(log, 0, strlen(log)+1);
		snprintf(log, 511, "memory[ %p ],file[ %s ], function[ %s ],line[ %d ], size[ %d] \n", it->first, it->second.file, it->second.function, it->second.line, it->second.size);

		if(EOF == fputs(log, m_fileHandle))
		{
			std::cerr << "MemoryLeaks::writeLog fputs error" << std::endl;
			return ;
		}

		len += strlen(log);
	}

	memset(log, 0, strlen(log)+1);
	snprintf(log, 511, "\n-----------------------------------memory leaks end-------------------------------\n\n");

	if(EOF == fputs(log, m_fileHandle))
	{
		std::cerr << "MemoryLeaks::writeLog fputs error" << std::endl;
		return ;
	}

	fflush(m_fileHandle);
	len += strlen(log);
	m_fileSize += len;
	if(m_fileSize >= 1024 * 1024 * 100)
	{
		CreateFile();
	}
}

int MemoryLeaks::svr()
{
	int iTick = 0;

	while(!m_flag)
	{
		if(iTick >= m_time)
		{
			iTick = 0;

//			WriteLog();
		}
		else
		{
			iTick += 1;
		}

		sleep(1);
	}

	return 0;
}


MemAllocator *MemAllocator::Allocator_ = 0;

void* MemAllocator::realloc(void* p,size_t  rbytes)
{
	char  flag = 0;

	p  = move_void_ptr(p,-1);
	CUtil::SafeMemmove(&flag, 1, p, 1);

	switch(flag)
	{
	case 0:
		if (rbytes < 16)
		{
			return p;
		}
		break;
	case 1:
		if (rbytes < 32)
		{
			return p;
		}
		break;
	case 2:
		if (rbytes < 64)
		{
			return p;
		}
		break;
	case 3:
		if (rbytes < 128)
		{
			return p;
		}
		break;
	case 4:
		if (rbytes < 256)
		{
			return p;
		}
		break;
	case 5:
		if (rbytes < 512)
		{
			return p;
		}
		break;
	case 6:
		if (rbytes < 1024)
		{
			return p;
		}
		break;
	case 7:
		if (rbytes < 2048)
		{
			return p;
		}
		break;
	case 8:
		if (rbytes < 4096)
		{
			return p;
		}
		break;
	case 9:
		if (rbytes < 8192)
		{
			return p;
		}
		break;
	case 10:
		if (rbytes < 16384)
		{
			return p;
		}
		break;
	case 11:
		if (rbytes < 32768)
		{
			return p;
		}
		break;
	case 12:
		if (rbytes < 65536)
		{
			return p;
		}
		break;
	case 13:
		return 0;
		break;

	default:
		break;
	}

	return 0;
}

void*  MemAllocator::malloc(size_t nbytes, const char *file, int line, const char *function)
{
	void* p = NULL;
	char flag;

	if (nbytes <= 16)
	{
		p = Allocator16_.malloc();
#ifdef USE_MEMORY_LEAKS
		MemoryLeaks::GetInstance()->AddLog(p, 16, file, line, function);
#endif
		flag =  0;
		CUtil::SafeMemmove(p, 1, &flag, 1);
		p = move_void_ptr(p,1);

		return p;
	}

	if (nbytes <= 32)
	{
		p = Allocator32_.malloc();
#ifdef USE_MEMORY_LEAKS
		MemoryLeaks::GetInstance()->AddLog(p, 32, file, line, function);
#endif
		flag =  1;
		CUtil::SafeMemmove(p, 1, &flag, 1);
		p = move_void_ptr(p,1);

		return p;
	}

	if (nbytes <= 64)
	{
		p = Allocator64_.malloc();
#ifdef USE_MEMORY_LEAKS
		MemoryLeaks::GetInstance()->AddLog(p, 64, file, line, function);
#endif
		flag =  2;
		CUtil::SafeMemmove(p, 1, &flag, 1);
		p = move_void_ptr(p,1);

		return p;
	}

	if (nbytes <= 128)
	{
		p = Allocator128_.malloc();
#ifdef USE_MEMORY_LEAKS
		MemoryLeaks::GetInstance()->AddLog(p, 128, file, line, function);
#endif
		flag =  3;
		CUtil::SafeMemmove(p, 1, &flag, 1);
		p = move_void_ptr(p,1);

		return p;
	}

	if (nbytes <= 256)
	{
		p = Allocator256_.malloc();
#ifdef USE_MEMORY_LEAKS
		MemoryLeaks::GetInstance()->AddLog(p, 256, file, line, function);
#endif
		flag =  4;
		CUtil::SafeMemmove(p, 1, &flag, 1);
		p = move_void_ptr(p,1);

		return p;
	}


	if (nbytes <= 512)
	{
		p = Allocator512_.malloc();
#ifdef USE_MEMORY_LEAKS
		MemoryLeaks::GetInstance()->AddLog(p, 512, file, line, function);
#endif
		flag =  5;
		CUtil::SafeMemmove(p, 1, &flag, 1);
		p = move_void_ptr(p,1);

		return p;
	}

	if(nbytes <= 1024)
	{
		p = Allocator1024_.malloc();
#ifdef USE_MEMORY_LEAKS
		MemoryLeaks::GetInstance()->AddLog(p, 1024, file, line, function);
#endif
		flag = 6;
		CUtil::SafeMemmove(p, 1, &flag, 1);
		p = move_void_ptr(p,1);

		return p;
	}

	if(nbytes <= 2048)
	{
		p = Allocator2048_.malloc();
#ifdef USE_MEMORY_LEAKS
		MemoryLeaks::GetInstance()->AddLog(p, 2048, file, line, function);
#endif
		flag = 7;
		CUtil::SafeMemmove(p, 1, &flag, 1);
		p = move_void_ptr(p,1);

		return p;
	}

	if (nbytes <= 4096)
	{
		p = Allocator4096_.malloc();
#ifdef USE_MEMORY_LEAKS
		MemoryLeaks::GetInstance()->AddLog(p, 4096, file, line, function);
#endif
		flag =  8;
		CUtil::SafeMemmove(p, 1, &flag, 1);
		p = move_void_ptr(p,1);

		return p;
	}

	if(nbytes <= 8192)
	{
		p = Allocator8192_.malloc();
#ifdef USE_MEMORY_LEAKS
		MemoryLeaks::GetInstance()->AddLog(p, 8192, file, line, function);
#endif
		flag = 9;
		CUtil::SafeMemmove(p, 1, &flag, 1);
		p = move_void_ptr(p,1);

		return p;
	}

	if (nbytes <= 16384)
	{
		p = Allocator16384_.malloc();
#ifdef USE_MEMORY_LEAKS
		MemoryLeaks::GetInstance()->AddLog(p, 16384, file, line, function);
#endif
		flag = 10;
		CUtil::SafeMemmove(p, 1, &flag, 1);
		p = move_void_ptr(p,1);

		return p;
	}

	if(nbytes <= 32768)
	{
		p = Allocator32768_.malloc();
#ifdef USE_MEMORY_LEAKS
		MemoryLeaks::GetInstance()->AddLog(p, 32768, file, line, function);
#endif
		flag = 11;
		CUtil::SafeMemmove(p, 1, &flag, 1);
		p = move_void_ptr(p,1);

		return p;
	}

	if (nbytes <= 65536)
	{
		p = Allocator65536_.malloc();
#ifdef USE_MEMORY_LEAKS
		MemoryLeaks::GetInstance()->AddLog(p, 65536, file, line, function);
#endif
		flag = 12;
		CUtil::SafeMemmove(p, 1, &flag, 1);
		p = move_void_ptr(p,1);

		return p;
	}

	p = ::malloc(nbytes + 1);
	if (p == NULL)
	{
		LOG_BASE(FILEINFO, "memory pool malloc error");
	}
#ifdef USE_MEMORY_LEAKS
	MemoryLeaks::GetInstance()->AddLog(p, nbytes, file, line, function);
#endif
	flag =  13;
	CUtil::SafeMemmove(p, 1, &flag, 1);
	p = move_void_ptr(p,1);

	return p;
}

void* MemAllocator::move_void_ptr(void* ptr,int i)
{
	char* p  = (char*)ptr;

	p = p + i;
	void* np = p;

	return np;
}

void MemAllocator::free(void* p)
{
	if(p == 0)
		return;

	char  flag = 0;

	p  = move_void_ptr(p,-1);
	CUtil::SafeMemmove(&flag, 1, p, 1);
	switch(flag)
	{
	case 0:
		Allocator16_.free(p);
		break;
	case 1:
		Allocator32_.free(p);
		break;
	case 2:
		Allocator64_.free(p);
		break;
	case 3:
		Allocator128_.free(p);
		break;
	case 4:
		Allocator256_.free(p);
		break;
	case 5:
		Allocator512_.free(p);
		break;
	case 6:
		Allocator1024_.free(p);
		break;
	case 7:
		Allocator2048_.free(p);
		break;
	case 8:
		Allocator4096_.free(p);
		break;
	case 9:
		Allocator8192_.free(p);
		break;
	case 10:
		Allocator16384_.free(p);
		break;
	case 11:
		Allocator32768_.free(p);
		break;
	case 12:
		Allocator65536_.free(p);
		break;
	case 13:
		::free(p);
		break;
	default:
		break;
	}

#ifdef USE_MEMORY_LEAKS
	MemoryLeaks::GetInstance()->DeleteLog(p);
#endif
}

void MemAllocator::free(void* p,size_t nbytes)
{
	if (nbytes <= 16)
	{
		Allocator16_.free(p);

		return;
	}

	if (nbytes <= 32)
	{
		Allocator32_.free(p);

		return;
	}

	if (nbytes <= 64)
	{
		Allocator64_.free(p);

		return;
	}

	if (nbytes <= 128)
	{
		Allocator128_.free(p);

		return;
	}

	if (nbytes <= 256)
	{
		Allocator256_.free(p);

		return;
	}

	if (nbytes <= 512)
	{
		Allocator512_.free(p);
		return;
	}

	if (nbytes <= 1024)
	{
		Allocator1024_.free(p);

		return;
	}

	if (nbytes <= 2048)
	{
		Allocator2048_.free(p);

		return;
	}

	if (nbytes <= 4096)
	{
		Allocator4096_.free(p);

		return;
	}

	if (nbytes <= 8192)
	{
		Allocator8192_.free(p);

		return;
	}

	if (nbytes <= 16384)
	{
		Allocator16384_.free(p);

		return;
	}

	if (nbytes <= 32768)
	{
		Allocator32768_.free(p);

		return;
	}

	if (nbytes <= 65536)
	{
		Allocator65536_.free(p);

		return;
	}

	::free(p);
}


void * MemoryBase::operator new (size_t size, const char *file, int line, const char *function)
{
	return CommBaseOut::MemAllocator::GetInstance()->malloc(size, file, line, function);
}
void * MemoryBase::operator new[] (size_t size, const char *file, int line, const char *function)
{
	return CommBaseOut::MemAllocator::GetInstance()->malloc(size, file, line, function);
}
void MemoryBase::operator delete (void * p, const char *file, int line, const char *function)
{
	CommBaseOut::MemAllocator::GetInstance()->free(p);
}
void MemoryBase::operator delete[] (void * p, const char *file, int line, const char *function)
{
	CommBaseOut::MemAllocator::GetInstance()->free(p);
}
void MemoryBase::operator delete[] (void * p)
{
	CommBaseOut::MemAllocator::GetInstance()->free(p);
}
void MemoryBase::operator delete (void * p)
{
	CommBaseOut::MemAllocator::GetInstance()->free(p);
}

}
