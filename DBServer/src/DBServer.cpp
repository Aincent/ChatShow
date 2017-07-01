//============================================================================
// Name        : DBServer.cpp
// Author      : Samuel
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "ServiceMain.h"
#include "util.h"

using namespace std;

int main()
{
#ifndef TEST
	CUtil::InitDaemon();

	signal(SIGUSR1,CServiceMain::Stop);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGINT,SIG_IGN);
#endif

	CServiceMain service;


	service.Start();


	return 0;
}
