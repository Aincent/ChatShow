/*
 * ServiceMain.cpp
 *
 *  Created on: Jun 28, 2017
 *      Author: root
 */
#include "ServiceMain.h"
#include <signal.h>
#include "./Log/Logger.h"
#include "./Network/Message_Facade.h"
#include "util.h"
#include "./Network/NetWorkConfig.h"
#include "ServerManager.h"
#include "./SqlInterface/SqlConnPool.h"
#include "Lua/LuaPool.h"
#include "Timer/TimerInterface.h"
#include "CharMemCache/CharMemCache.h"
#include "CharactorLogin/CharLogin.h"

CServiceMain::CServiceMain()
{
}

CServiceMain::~CServiceMain()
{
}

int CServiceMain::Init()
{
	std::string strFileFullPath;
	std::string strLuaPath;

	if(CUtil::GetModulePath(strFileFullPath) != 0)
	{
		return -1;
	}

	string loadConfig = strFileFullPath + "Config/DBServerConfig.lua";

	if(m_conf.LoadConfig(loadConfig))
	{
		return -1;
	}

	return 0;
}

int CServiceMain::InitInstance()
{
	CCharLogin::GetInstance();
	CCharMemCache::GetInstance();
	return 0;
}

void CServiceMain::UnInitInstance()
{
	CCharLogin::GetInstance()->DestoryInstance();
	CCharMemCache::GetInstance()->DestroyInstance();
	ServerConHandler::GetInstance()->DestoryInstance();
	CSqlConnPool::GetInstance()->DestroyInstance();
	LuaPool::GetInstance()->DestroyInstance();
	TimerInterface::GetInstance()->DestoryInstance();
	CLoggerMT::GetInstance()->DestoryInstance();

#ifdef USE_MEMORY_LEAKS
	MemoryLeaks::GetInstance()->DestroyInstance();
#endif

#ifdef USE_MEMORY_POOL
	MemAllocator::GetInstance()->DestroyInstance();
#endif

}

void CServiceMain::Start()
{
#ifdef USE_MEMORY_LEAKS
	MemoryLeaks::GetInstance()->Init(60);
#endif

#ifdef USE_MEMORY_POOL
	MemAllocator::GetInstance();
#endif

	if(Init())
	{
		cout << " init config error " << endl;

		return ;
	}

	if(CLoggerMT::GetInstance()->Init(m_conf.GetLogConf().fileLen, m_conf.GetLogConf().info, m_conf.GetLogConf().debug,
			m_conf.GetLogConf().warning, m_conf.GetLogConf().error, m_conf.GetLogConf().fatal, m_conf.GetLogConf().display,
			m_conf.GetLogConf().filePath, m_conf.GetLogConf().module) != 0)
	{
		cout << " init log error " << endl;

		return ;
	}

	if(CSqlConnPool::GetInstance()->Init(m_conf.GetDBConf().host, m_conf.GetDBConf().user, m_conf.GetDBConf().passwd,
			m_conf.GetDBConf().db, m_conf.GetDBConf().port, m_conf.GetBlockThread() * 4, m_conf.GetDBConf().rate) != 0)
	{
		cout <<endl<< "Mysql Init Failed ,host is "<<m_conf.GetDBConf().host<<",user is "<<m_conf.GetDBConf().user<<",password is "<<m_conf.GetDBConf().passwd<<",db is "<<m_conf.GetDBConf().db<<",port is "<<m_conf.GetDBConf().port<< endl;

		return ;
	}

	TimerInterface::GetInstance()->Init(eTimerThreadSafe);

	if(Message_Facade::Init(ServerConHandler::GetInstance(), m_conf.GetBlockThread(), m_conf.GetIOThread()))
	{
		cout << " init net error "  <<endl;

		return;
	}

	ServerConHandler::GetInstance()->RegisterMsg();
	ServerConHandler::GetInstance()->SetSvrID(m_conf.GetServerID());

	if(InitInstance())
	{
		return ;
	}

//	CCharLogin::GetInstance()->InitCharTemplate();

	vector<AcceptorConfig> acc = m_conf.GetAccCof();
	vector<AcceptorConfig>::iterator itAcc = acc.begin();
	for(; itAcc!=acc.end(); ++itAcc)
	{
		AcceptorConfig accConf = *itAcc;
		Message_Facade::AddAcceptConfig(accConf);
	}

	vector<ConnectionConfig> con = m_conf.GetConnCof();
	vector<ConnectionConfig>::iterator itCon = con.begin();
	for(; itCon!=con.end(); ++itCon)
	{
		ConnectionConfig connConf = *itCon;
		Message_Facade::AddConnectionConfig(connConf);
	}

	if(Message_Facade::Run())
	{
		cout << " run net error "  <<endl;

		return;
	}


//	sleep(15 * 60);
	Message_Facade::Wait();
	Message_Facade::Stop();

	UnInitInstance();
}

void CServiceMain::Stop(int signal)
{
	Message_Facade::UnWait();
}

