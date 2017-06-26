/*
 * Password.cpp
 *
 *  Created on: 18/06/2015
 *      Author: root
 */

#include "Password.h"

//soureStr为密码，targetStr为md5码
string Password::makeStr(string soureStr,string targetStr)
{
	string tempResult;
	int tempCounts = 0;
	while(soureStr.size() > 0 || targetStr.size() > 0)
	{
		//密码为0、2、4..位，更长部份的直接在往面加
		if(tempCounts % 2 == 0)
		{
			if(soureStr.size() > 0)
			{
				tempResult += *soureStr.begin() + 1;
				soureStr.erase(soureStr.begin());
			}
			else if(targetStr.size() > 0)
			{
				tempResult += *targetStr.begin();
				targetStr.erase(targetStr.begin());
			}

		}
		else if(targetStr.size() > 0)
		{
			if(targetStr.size() > 0)
			{
				tempResult += *targetStr.begin();
				targetStr.erase(targetStr.begin());
			}
			else if(soureStr.size() > 0)
			{
				tempResult += *soureStr.begin() + 1;
				soureStr.erase(soureStr.begin());
			}
		}

		++tempCounts;
	}

	return tempResult;
}

string Password::getStr(string& str)
{
	string tempResult;
	int tempSize = str.size();
	//MD5码只有32位
	int tempCounts = abs(tempSize - 32);
	for (int i = 0;i < tempCounts;++i)
	{
		int tempIndex = 0;
		if(i >= 32)
			tempIndex = 32 + i;
		else
			tempIndex = i * 2;

		if(tempIndex >= tempSize)
			break;

		tempResult += str[tempIndex] - 1;
	}

	return tempResult;

}
