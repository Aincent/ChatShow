//============================================================================
// Name        : GateServer.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "ServiceMain.h"
#include "util.h"

using namespace std;
using namespace std;

int main()
{
#ifndef TEST
	CUtil::InitDaemon();

	signal(SIGUSR1,CServiceMain::Stop);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGINT,SIG_IGN);
#endif

#ifdef USE_MEMORY_LEAKS
	MemoryLeaks::GetInstance()->Init(60);
#endif

#ifdef USE_MEMORY_POOL
	MemAllocator::GetInstance();
#endif

	{
		CServiceMain service;

		service.Start();
	}

#ifdef USE_MEMORY_LEAKS
	MemoryLeaks::GetInstance()->DestroyInstance();
#endif

#ifdef USE_MEMORY_POOL
	MemAllocator::GetInstance()->DestroyInstance();
#endif

	return 0;
}

