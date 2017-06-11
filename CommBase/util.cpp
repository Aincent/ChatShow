/*
 * util.cpp
 *
 *  Created on: Jun 8, 2017
 *      Author: root
 */

#include "util.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <unistd.h>

namespace CommBaseOut
{
unsigned char g_utf8Table[] = {
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,

4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
};

CUtil::CUtil()
{
}

CUtil::~CUtil()
{
}

int CUtil::SafeMemmove(void *pDst, DWORD dwDstLen, const void *pSrc, DWORD dwCopyLen)
{
	if(dwDstLen < dwCopyLen)
	{
        std::cerr << "CUtil::SafeMemmove():error parameters" << std::endl;
		return -1;
	}
	memmove(pDst, pSrc, dwCopyLen);
    return 0;
}

DWORD64 CUtil::ntohll(DWORD64 src)
{
	unsigned char des[16] = {0};
	DWORD64 dwDst = -1;

	des[7] = (unsigned char)(src & 0xff);
	des[6] = (unsigned char)(src >> 8 & 0xff);
	des[5] = (unsigned char)(src >> 16 & 0xff);
	des[4] = (unsigned char)(src >> 24 & 0xff);
	des[3] = (unsigned char)(src >> 32 & 0xff);
	des[2] = (unsigned char)(src >> 40 & 0xff);
	des[1] = (unsigned char)(src >> 48 & 0xff);
	des[0] = (unsigned char)(src >> 56 & 0xff);

	memcpy(&dwDst, des, 8);

	return dwDst;
}

int CUtil::StringToInt(const std::string &str)
{
    return atoi(str.c_str());
}

int CUtil::ByteToStr(const BYTE *buf, DWORD dwLen, std::string &str)
{
    char strTemp[4] = "";
    for(DWORD i = 0; i < dwLen; i++)
    {
        if(snprintf(strTemp, 4, "%02x ", buf[i]) < 0)
        {
            std::cerr << "CUtil::ByteToStr():snprintf -1!" << std::endl;
            return -1;
        }
        str += std::string(strTemp);
    }
    return 0;
}

int CUtil::GetBinPath(std::string &strFileFullPath)
{
    char strTemp[PATH_MAX] = "";
    int nLen = readlink("/proc/self/exe", strTemp, PATH_MAX);
    if(nLen < 0)
    {
        std::cerr << "CUtil::GetBinPath():readlink -1!" << std::endl;
        return -1;
    }
    strFileFullPath = std::string(strTemp);
    return 0;
}


int CUtil::GetModulePath(std::string &strFileFullPath)
{
    if(GetBinPath(strFileFullPath) != 0)
    {
        std::cerr << "CUtil::GetModulePath():GetBinPath -1!" << std::endl;
        return -1;
    }
    strFileFullPath.erase(strFileFullPath.rfind("/") + 1, strFileFullPath.length());
    return 0;
}

int CUtil::GetFullPath(const std::string &strFilePath, std::string &strFullPath)
{
	if(strFilePath[0] == '/')
    {
        strFullPath = strFilePath;
        return 0;
    }

	if(CUtil::GetModulePath(strFullPath) != 0)
	{
        std::cerr << "CUtil::GetFullPath():GetModulePath -1!" << std::endl;
		return -1;
	}

    strFullPath += strFilePath;
	return 0;
}

int CUtil::GetFullPathFromRelative(const std::string &strFilePath, std::string &strConvertPath)
{
	std::string strExePath;
	if (CUtil::GetModulePath(strExePath) != 0)
	{
		return -1;
	}

	int nSubSize = 0;
	const char *strBackPre = "..";

	std::size_t found = strFilePath.find(strBackPre);
	while (found != std::string::npos)
	{
		nSubSize++;
		found = strFilePath.find(strBackPre, found + 3);
	}

	if (0 == nSubSize)
	{
		std::cout << "invalid back pre!" << std::endl;
		return -1;
	}

	found = strExePath.length()-1;
	while (nSubSize > 0)
	{
		found = strExePath.rfind('/', found - 1);
		if (found == std::string::npos)
		{
			break;
		}

		nSubSize--;
	}

	if (nSubSize > 0)
	{
		std::cerr << "not find right back pre!" << std::endl;
		return -1;
	}

	strConvertPath = strExePath.substr(0, found) + strFilePath.substr(strFilePath.rfind(strBackPre) + 2);
	return 0;
}

int64 CUtil::GetFileSize(const std::string &strFileName)
{
    std::string strFullPath;
    if(GetFullPath(strFileName, strFullPath) != 0)
    {
        std::cerr << "CUtil::GetFileSize():GetFullPath -1!" << std::endl;
        return -1;
    }
    struct stat buf;
    if(stat(strFullPath.c_str(), &buf) < 0)
    {
        std::cerr << "CUtil::GetFileSize():stat -1!" << std::endl;
        return -1;
    }
    return buf.st_size;
}

int64 CUtil::GetFolderSize(const std::string &strFilePath)
{
    std::string strFullPath;
    if(GetFullPath(strFilePath, strFullPath) != 0)
    {
        std::cerr << "CUtil::GetFolderSize():GetFullPath -1!" << std::endl;
        return -1;
    }

    DIR *pDir;
    struct dirent *pDirent;
    DWORD64 size = 0;
    if((pDir = opendir(strFullPath.c_str())) == NULL)
    {
        std::cerr << "CUtil::GetFolderSize():opendir -1!" << std::endl;
        return -1;
    }

    while((pDirent = readdir(pDir)) != NULL)
    {
        if(strncmp(pDirent->d_name, ".", PATH_MAX) == 0 || strncmp(pDirent->d_name, "..", PATH_MAX) == 0)
        {
            continue;
        }

        std::string strTemp;
        if(pDirent->d_type == DT_DIR)
        {
            if(strFullPath[strFullPath.length() - 1] != '/')
            {
                strFullPath += "/";
            }
            strTemp = strFullPath + std::string(pDirent->d_name);
            int64 nRet = GetFolderSize(strTemp);
            if(nRet < 0)
            {
                std::cerr << "CUtil::GetFolderSize:GetFolderSize -1!" << std::endl;
                closedir(pDir);
                return -1;
            }
            size += nRet;
        }
        else if(pDirent->d_type == DT_REG)
        {
            int nRet = GetFileSize(strTemp);
            if(nRet < 0)
            {
                std::cerr << "CUtil::GetFolderSize:GetFileSize -1!" << std::endl;
                closedir(pDir);
                return -1;
            }
            size += nRet;
        }
    }
    closedir(pDir);
    return size;
}

void CUtil::GetSpanDate(int nSpan, std::string &strDate)
{
	time_t timeNow = time(NULL);
	time_t timeSpan = timeNow + nSpan * SECONDS_OF_DAY;

	struct tm *tmSpan = localtime(&timeSpan);

    char szDate[20] = "";
	snprintf(szDate, 20, "%4d-%02d-%02d", tmSpan->tm_year + 1900, tmSpan->tm_mon + 1, tmSpan->tm_mday);
    strDate = std::string(szDate);
}

void CUtil::GetSpanMinutes(std::string &strDate)
{
	time_t timeNow = time(NULL);
	struct tm *tmSpan = localtime(&timeNow);
	char szDate[16] = "";

	snprintf(szDate, 20, "%2d%02d%02d%02d%02d", tmSpan->tm_year  - 100, tmSpan->tm_mon + 1, tmSpan->tm_mday, tmSpan->tm_hour, tmSpan->tm_min);
	strDate = std::string(szDate);
}

DWORD64 CUtil::GetNowSecond()
{
	struct timeval time;

	gettimeofday(&time, NULL);
	DWORD64 se = ((DWORD64)time.tv_sec)*1000+(DWORD64)time.tv_usec/1000;

	return se;
}

DWORD64 CUtil::GetNowMicrosecod()
{
	struct timeval time;

	gettimeofday(&time, NULL);
	DWORD64 se = ((DWORD64)time.tv_sec)*1000*1000+(DWORD64)time.tv_usec;

	return se;
}

BYTE CUtil::GetCurMonthDay(int year, int month)
{
	switch(month)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		{
			return 31;
		}
	case 4:
	case 6:
	case 9:
	case 11:
		{
			return 30;
		}
	case 2:
		{
			if((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
			{
				return 29;
			}

			break;
		}
	default :
		{
			return 0;
		}
	}

	return 28;
}

int64 CUtil::GetNextMonthDay(int year, int month, int day, int &nMonth)
{
	int64 diffTime = 0;
	int j=1;
	int tyear = year;

	for(int i=month; i<=12 && j<= 12; ++i, ++j)
	{
		int tmon = i + 1;
		if(tmon > 12)
		{
			tmon = 1;
			tyear = year + 1;
		}

		int tday = (int64)GetCurMonthDay(tyear, tmon);
		if(tday >= day)
		{//下个月的天数比当前的日期大或者等于，那么就出循环
			nMonth = tmon;

			break;
		}
		else
		{
			diffTime += (int64)tday *  24 * 3600 * 1000;
		}
	}

	if(j > 12)
	{
		diffTime = 0;
	}
	else
	{
		diffTime += (int64)GetCurMonthDay(year, month) *  24 * 3600 * 1000;
	}

	return diffTime;
}

bool CUtil::IsSameDay(int64 btime)
{
	time_t t = btime / (int64)1000;
	time_t now;
	struct tm pNow;
	struct tm p;

	int nowyear = 0;
	int year = 0;
	int nowmon = 0;
	int mon = 0;
	int nowday = 0;
	int day = 0;

	time(&now);
	localtime_r(&now, &pNow);
	nowyear = pNow.tm_year;
	nowmon = pNow.tm_mon;
	nowday = pNow.tm_mday;
	localtime_r(&t, &p);
	year = p.tm_year;
	mon = p.tm_mon;
	day = p.tm_mday;

	if( nowyear != year || nowmon != mon || nowday != day )
	{
		return false;
	}

	return true;
}

bool CUtil::IsSameWeek(int64 btime)
{
	time_t t = btime / (int64)1000;
	time_t now;
	struct tm pNow;
	struct tm p;
	int nowyear = 0;
	int year = 0;
	int nowmon = 0;
	int mon = 0;
	int nowweek = 0;
	int week = 0;
	int nowday = 0;
	int day = 0;

	time(&now);
	localtime_r(&now, &pNow);
	nowyear = pNow.tm_year;
	nowmon = pNow.tm_mon;
	nowday = pNow.tm_mday;
	nowweek = pNow.tm_wday;
	if(nowweek == 0)
		nowweek = 7;
	localtime_r(&t, &p);
	year = p.tm_year;
	mon = p.tm_mon;
	day = p.tm_mday;
	week = p.tm_wday;
	if(week == 0)
		week = 7;

	if( nowyear != year )
	{
		return false;
	}
	else
	{
		if( nowmon == mon )
		{
			if( abs(nowday - day) >= 7 )
				return false;

			if( nowday > day )
			{
				if( nowweek < week )
				{
					return false;
				}
			}
			else if( nowday < day )
			{
				if( nowweek > week )
				{
					return false;
				}
			}
		}
		else
		{
			if( abs(nowmon - mon) == 1 )
			{
				if( nowmon > mon )
				{
					if( ( GetCurMonthDay( year, mon ) - day + nowday ) >= 7 )
					{
						return false;
					}

					if( nowweek < week )
					{
						return false;
					}
				}
				else
				{
					if( ( GetCurMonthDay( nowyear, nowmon ) - nowday + day ) >= 7 )
					{
						return false;
					}

					if( nowweek < week )
					{
						return false;
					}
				}
			}
			else
			{
				return false;
			}
		}
	}

	return true;
}

bool CUtil::IsSameMonth(int64 btime)
{
	time_t t = btime / (int64)1000;
	time_t now;
	struct tm pNow;
	struct tm p;
	int nowyear = 0;
	int year = 0;
	int nowmon = 0;
	int mon = 0;

	time(&now);
	localtime_r(&now, &pNow);
	nowyear = pNow.tm_year;
	nowmon = pNow.tm_mon;
	localtime_r(&t, &p);
	year = p.tm_year;
	mon = p.tm_mon;

	if( nowyear != year || nowmon != mon )
	{
		return false;
	}

	return true;
}

DWORD64 CUtil::TimeToPoint(int year, int month, int day, int hour, int minutes, int seconds)
{
	DWORD64 point = 0;
	struct tm timeNow;
	time_t t;

	timeNow.tm_year = year - 1900;
	timeNow.tm_mon = month - 1;
	timeNow.tm_mday = day;
	timeNow.tm_hour = hour;
	timeNow.tm_min = minutes;
	timeNow.tm_sec = seconds;

	t = mktime(&timeNow);
	point = (DWORD64)t * 1000;

	return point;
}

void CUtil::PointToTime(DWORD64 point, int &year, int &month, int &day, int &hour, int &minutes, int &seconds)
{
	struct tm p;
	time_t t = point / (DWORD64)1000;

	localtime_r(&t, &p);

	year = p.tm_year + 1900;
	month = p.tm_mon + 1;
	day = p.tm_mday;
	hour = p.tm_hour;
	minutes = p.tm_min;
	seconds = p.tm_sec;
}

int CUtil::GetClientIp(int sock, std::string &strIp)
{
	struct sockaddr_in addr;
	int nLen = sizeof(struct sockaddr_in);

	if(getpeername(sock, (struct sockaddr *)&addr, (socklen_t *)&nLen) < 0)
	{
		std::cerr << "CUtil::GetIp():getpeername -1!" << std::endl;
		return -1;
	}
	strIp = std::string(inet_ntoa(addr.sin_addr));
	return 0;
}

void CUtil::InitDaemon()
{
    DaemonFork();
	setsid();
    DaemonFork();
	int nFdMax = getdtablesize();

	for(int i = 3; i < nFdMax; ++i)
	{
		close(i);
	}

	//chdir("/root/bin");
	umask(0);
	return;
}

void CUtil::DaemonFork()
{
    int nPid = fork();
    if(nPid < 0)
    {
        exit(1);
    }
    else if(nPid != 0)
    {
        exit(0);
    }
}

int CUtil::GetIpName(int nSocket, char strIpName[IPSTR_LEN])
{
	if (nSocket <= -1)
	{
		return -1;
	}

	struct sockaddr_in peeraddr;
	socklen_t len = sizeof(peeraddr);
	int ret = getpeername(nSocket, (struct sockaddr*)&peeraddr, &len);
	if (ret < 0)
	{
		return -1;
	}

	if (NULL == inet_ntop(AF_INET, &peeraddr.sin_addr, strIpName, IPSTR_LEN))
	{
		return -1;
	}

	return 0;
}

std::string CUtil::GetIpByName(int socket, std::string &name)
{
	struct ifreq ifr;
	struct sockaddr_in sin;
	std::string ip;

	strncpy(ifr.ifr_name, name.c_str(), name.size());
	strncpy(ifr.ifr_name + name.size(), "\0", 1);
	if(ioctl(socket, SIOCGIFADDR, &ifr) < 0)
	{
		return "";
	}

	SafeMemmove(&sin, sizeof(sin), &ifr.ifr_addr, sizeof(ifr.ifr_addr));
	ip = inet_ntoa(sin.sin_addr);

	return ip;
}

std::string CUtil::GetIpByName(std::string &name)
{
	struct ifreq ifr;
	struct sockaddr_in sin;
	std::string ip;

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0)
	{
		return "";
	}

	strncpy(ifr.ifr_name, name.c_str(), name.size());
	strncpy(ifr.ifr_name + name.size(), "\0", 1);
	if(ioctl(sock, SIOCGIFADDR, &ifr) < 0)
	{
		return "";
	}

	SafeMemmove(&sin, sizeof(sin), &ifr.ifr_addr, sizeof(ifr.ifr_addr));
	ip = inet_ntoa(sin.sin_addr);

	return ip;
}

int CUtil::GetCPUCount()
{
	int count = 0;

	count = sysconf(_SC_NPROCESSORS_CONF);
	if(count <= 0)
		count = 1;

	return count;
}

int CUtil::GetUTF8Count(const char * str)
{
	if(NULL == str)
		return 0;

	int clen = strlen(str);
	int len = 0;

	for(const char *ptr = str; *ptr!=0&&len<clen; len+=(int)g_utf8Table[(unsigned char)*ptr], ptr+=g_utf8Table[(unsigned char)*ptr]);

	return len;
}

void CUtil::MSleep(int time)
{
	struct timeval tv = {0,1000 * time};
	select(0, 0, 0, 0, &tv);
}

}
