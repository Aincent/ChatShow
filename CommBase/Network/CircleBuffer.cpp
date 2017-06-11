/*
 * CircleBuffer.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "CircleBuffer.h"
#include "../Common.h"
#include "../util.h"
#include "../Log/Logger.h"

namespace CommBaseOut
{

void CCircleBuff::HadRead(int readLen)
{
	m_readIndex += readLen;
	if (m_readIndex > m_lastIndex)
	{
		LOG_BASE(FILEINFO, "circle buff read index beyond last index");
		//ERRORLOG("fatal buf! read over buff! %d, %d, %d", m_readIndex, m_writeIndex, readLen);
	}
}

void CCircleBuff::HadWrite(int writeLen)
{
	m_writeIndex += writeLen;
	if (m_writeIndex > m_bufCapacity)
	{
		LOG_BASE(FILEINFO, "circle buff write index beyond Capacity");
		//ERRORLOG("fatal buf! write over buff! %d, %d, %d", m_writeIndex, writeLen);
	}
}

void CCircleBuff::IsReadOverAgain()
{
	if (IsNormal())
	{
		return;
	}

	if (m_readIndex >= m_lastIndex)
	{
		m_readIndex = 0;
		m_lastIndex = m_bufCapacity;
	}

	return;
}

void CCircleBuff::IsWriteOverAgain(int writeLen)
{
	if (!IsNormal())
	{
		return;
	}

	if (GetWriteLen() < writeLen && m_readIndex > 1 && m_readIndex <= m_writeIndex)
	{
		m_lastIndex = m_writeIndex;
		m_writeIndex = 0;
	}

	return;
}

int CCircleBuff::WriteSend(const char * strBuff, const packetHeader *gh)
{
	int len = gh->length;

	if(gh->remoteType <= 0)
		len = htonl(gh->length);

	return WriteData(strBuff, len, gh, HEADER_LENGTH);
}

/*int CCircleBuff::WriteSendClient(const char * strBuff, const packetHeader &gh)
{
	return WriteData(strBuff, gh.length, ((char *)(&gh) + (INT_LEN << 1)), HEADER_LEN);
}*/

int CCircleBuff::WriteData(const void * strBuff, int nBuffLen, const void *pHead, int nHeadLen)
{
	const int writeLen = nBuffLen + nHeadLen;
	IsWriteOverAgain(writeLen);

	int spaceLen = GetWriteLen();
	if (spaceLen < writeLen)
	{
		LOG_BASE(FILEINFO, "write data but space less than write spacelen[%d] writelen[%d] ", spaceLen, writeLen);

		return eNoEnughWrite;
	}

	char *writeHead = GetWirte();

	if (eNetSuccess != CUtil::SafeMemmove(writeHead, spaceLen, pHead, nHeadLen))
	{
		return eWriteHeaderErr;
	}

	if (eNetSuccess != CUtil::SafeMemmove(writeHead + nHeadLen, spaceLen - nHeadLen, strBuff, nBuffLen))
	{
		return eWritePacketErr;
	}

	HadWrite(writeLen);

	return eNetSuccess;
}


int CCircleBuff::WriteBuffer(const char *strBuff, int buffLen)
{
	IsWriteOverAgain(buffLen);

	int spaceLen = GetWriteLen();
	if (spaceLen < buffLen)
	{
		return eNoEnughWrite;
	}

	char *writeHead = GetWirte();
	if (eNetSuccess != CUtil::SafeMemmove(writeHead, spaceLen, strBuff, buffLen))
	{
		return eWriteBuffErr;
	}

	HadWrite(buffLen);

	return eNetSuccess;
}

int CCircleBuff::ReadSendBuff(const char * &sendBuff, int &nbuffLen)
{
	IsReadOverAgain();
	if (IsEmpty())
	{
		return eBuffNULL;
	}

	sendBuff = GetRead();
	nbuffLen = GetReadLen();

	return eNetSuccess;
}

/*int CCircleBuff::CloneBuffer(const int nCapaticy)
{
	if (nCapaticy <= m_bufCapacity)
	{
		return FAILED;
	}

	char *pNewBuff = new char[nCapaticy + 1];
	int nWriteIndex = 0;

	const int savedIndex = SavedReadIndex();
	while (!IsEmpty())
	{
		int nbuffLen = 0;
		const char *sendBuff = NULL;

		if (ReadSendBuff(sendBuff, nbuffLen) != OK)
		{
			WARNINGLOG("clone buff read failed! %d, %d", savedIndex, nWriteIndex);
			RecoverReadIndex(savedIndex);
			SAFE_DELETE_ARRAY(pNewBuff)
			return FAILED;
		}

		if (OK != CUtil::SafeMemmove(pNewBuff + nWriteIndex, nCapaticy - nWriteIndex, sendBuff, nbuffLen))
		{
			WARNINGLOG("clone buff write failed! %d, %d", savedIndex, nWriteIndex);
			RecoverReadIndex(savedIndex);
			SAFE_DELETE_ARRAY(pNewBuff)
			return FAILED;
		}

		nWriteIndex += nbuffLen;
	}

	SAFE_DELETE_ARRAY(m_buffer)
	m_buffer = pNewBuff;

	m_readIndex = 0;
	m_writeIndex = nWriteIndex;
	m_lastIndex = nCapaticy;
	m_bufCapacity = nCapaticy;
	return OK;
}*/

}
