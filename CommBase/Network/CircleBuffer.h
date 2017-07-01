/*
 * CircleBuffer.h
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */

#ifndef NETWORK_CIRCLEBUFFER_H_
#define NETWORK_CIRCLEBUFFER_H_

#include "NetWorkConfig.h"
#include "../Memory/MemAllocator.h"

namespace CommBaseOut
{

class CSimLock;

class CCircleBuff
#ifdef USE_MEMORY_POOL
: public MemoryBase
#endif
{
public:
	CCircleBuff(int buffSize)
	{
		m_bufCapacity = buffSize;
		m_buffer = (char *)malloc(buffSize * sizeof(char));//new char[buffSize];// char[buffSize + 1]; //��һ����λ

		Reset();
	}

	 ~CCircleBuff(void)
	{
		free(m_buffer);
		m_buffer = 0;
	}

	 void ResetSize(int buffSize)
	 {
			m_bufCapacity = buffSize;
			m_buffer = (char *)realloc(m_buffer, buffSize * sizeof(char)); // char[buffSize];// char[buffSize + 1]; //��һ����λ

			Reset();
	 }

	inline void Reset()
	{
		m_readIndex = 0;
		m_writeIndex = 0;
		m_lastIndex = Size();
	}

	inline bool IsEmpty() const { return m_readIndex == m_writeIndex; }
	inline int GetReadLen() const
	{
		return IsNormal() ? (m_writeIndex - m_readIndex) : (m_lastIndex - m_readIndex);
	}

	inline int GetLeftBuffSize() const
 	{
		return IsNormal() ? (m_writeIndex - m_readIndex) : (m_lastIndex - m_readIndex + m_writeIndex);
	}

	inline int GetWriteLen()
	{
		if (IsNormal())
		{
			return Size() - m_writeIndex;
		}

		return m_readIndex - m_writeIndex - 1;
	}

	void HadRead(int readLen);
	void HadWrite(int writeLen);

	int ReadSendBuff(const char * &sendBuff, int &nbuffLen);

	int WriteSend(const char * strBuff, const packetHeader *gh);
	//int WriteSendClient(const char * strBuff, const packetHeader &gh);

	int WriteBuffer(const char *strBuff, int buffLen);

	//int CloneBuffer(const int nCapaticy);

	inline int SavedReadIndex() { return m_readIndex; }
	inline void RecoverReadIndex(int savedReadIndex) { m_readIndex = savedReadIndex; }

	//inline pthread_mutex_t& GetMutex() { return m_mutexBuff; }

	int GetReadIndex()
	{
		return m_readIndex;
	}

	int GetWriteIndex()
	{
		return m_writeIndex;
	}

	int GetLastIndex()
	{
		return m_lastIndex;
	}

private:

//	CCircleBuff(const CCircleBuff&);
//	void operator=(const CCircleBuff&);

	inline char* Begin() { return m_buffer; }
	inline const char* Begin() const { return m_buffer; }
	inline int Size() const { return m_bufCapacity; }

	inline bool IsNormal() const { return m_readIndex <= m_writeIndex; }
	void IsWriteOverAgain(int writeLen);
	void IsReadOverAgain();

	inline const char* GetRead() const { return Begin() + m_readIndex; }
	inline char* GetWirte() { return Begin() + m_writeIndex;	}

	int WriteData(const void * strBuff, int nBuffLen, const void *pHead, int nHeadLen);

	volatile int m_readIndex;
	volatile int m_writeIndex;
	volatile int m_lastIndex;
	volatile int m_bufCapacity;

	char *m_buffer;
	//CSimLock m_mutexBuff;
};
}

#endif /* NETWORK_CIRCLEBUFFER_H_ */
