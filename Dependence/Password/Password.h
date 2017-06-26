/*
 * Password.h
 *
 *  Created on: 18/06/2015
 *      Author: root
 */

#ifndef PASSWORD_H_
#define PASSWORD_H_

#include "define.h"
using namespace std;

class Password
{
public:
	static string makeStr(string soureStr,string targetStr);

	static string getStr(string& str);
};


#endif /* PASSWORD_H_ */
