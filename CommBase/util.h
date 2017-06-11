/*
 * util.h
 *
 *  Created on: Jun 8, 2017
 *      Author: root
 */

#ifndef UTIL_H_
#define UTIL_H_

#pragma once
#include "define.h"

namespace CommBaseOut
{
class CUtil
{
public:
	CUtil();
	~CUtil();

	static int SafeMemmove(void *pDst, DWORD dwDstLen, const void *pSrc, DWORD dwCopyLen);

	static DWORD64 ntohll(DWORD64 src);

	static int StringToInt(const std::string &str);

	static int ByteToStr(const BYTE *buf, DWORD dwLen, std::string &str);

	static int GetBinPath(std::string &strFileFullPath);

	static int GetModulePath(std::string &strFileFullPath);

	static int GetFullPath(const std::string &strFilePath, std::string &strFullPath);

	static int GetFullPathFromRelative(const std::string &strFilePath, std::string &strConvertPath);

	static int64 GetFileSize(const std::string &strFileName);

	static int64 GetFolderSize(const std::string &strFilePath);

	static void GetSpanDate(int nSpan, std::string &strDate);

	static void GetSpanMinutes(std::string &strDate);

	static DWORD64 GetNowSecond();

	static DWORD64 GetNowMicrosecod();

	static BYTE GetCurMonthDay(int year, int month);

	static int64 GetNextMonthDay(int year, int month, int day, int &nMonth);

	static bool IsSameDay(int64 btime);

	static bool IsSameWeek(int64 btime);

	static bool IsSameMonth(int64 btime);

	static DWORD64 TimeToPoint(int year, int month, int day, int hour, int minutes, int seconds);

	static void PointToTime(DWORD64 point, int &year, int &month, int &day, int &hour, int &minutes, int &seconds);

	static int GetClientIp(int sock, std::string &strIp);

	static void InitDaemon();

	static int GetIpName(int nSocket, char strIpName[IPSTR_LEN]);

	static std::string GetIpByName(int socket, std::string &name);

	static std::string GetIpByName(std::string &name);

	static int GetCPUCount();

	static int GetUTF8Count(const char * str);

	static void MSleep(int time);

private:
	static void DaemonFork();
};

}

#endif /* UTIL_H_ */
