/*
 * ServiceMain.cpp
 *
 *  Created on: Jun 24, 2017
 *      Author: root
 */
#include "ServiceMain.h"
#include <signal.h>
#include "./Log/Logger.h"
#include "./Network/Message_Facade.h"
#include "util.h"
#include "./Network/NetWorkConfig.h"
#include "./Timer/TimerInterface.h"
#include "ServerManager.h"
#include "Object/Player/PlayerMessage.h"

bool CServiceMain::m_flag = false;

CServiceMain::CServiceMain()
{
}

CServiceMain::~CServiceMain()
{
}

int CServiceMain::Init()
{
	std::string strFileFullPath;
	std::string strFullPath;

	if(CUtil::GetModulePath(strFileFullPath) != 0)
	{
		return -1;
	}

	strFullPath = strFileFullPath + "Config/GameServerConfig.lua";

	if(m_conf.LoadConfig(strFullPath))
	{
		return -1;
	}

	return 0;
}


int CServiceMain::InitFile()
{
	std::string strFileFullPath;

	if(CUtil::GetModulePath(strFileFullPath) != 0)
	{
		return -1;
	}

	return 0;
}

int CServiceMain::InitInstance()
{
	PlayerMessage::GetInstance();
	return 0;
}

void CServiceMain::UnInit()
{
	PlayerMessage::GetInstance()->DestroyInstance();
//	GlobalEvent::GetInstance()->DestroyInstance();
//	CThreadPool::GetInstance()->DestroyInstance();
	TimerInterface::GetInstance()->DestoryInstance();
	CLoggerMT::GetInstance()->DestoryInstance();
	google::protobuf::ShutdownProtobufLibrary();
}


void CServiceMain::Start()
{
	#ifndef TEST
	signal(SIGUSR1,CServiceMain::Stop);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGINT,SIG_IGN);
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

	TimerInterface::GetInstance()->Init();
	//CThreadPool::GetInstance()->InitThread(m_conf.GetIOThread());

	if(Message_Facade::Init(ServerConHandler::GetInstance(), m_conf.GetBlockThread(), m_conf.GetIOThread()))
	{
		LOG_ERROR(FILEINFO, " init net error ");

		return;
	}

	if(InitFile() != 0)
	{
		return ;
	}

	if(InitInstance())
	{
		return;
	}

	ServerConHandler::GetInstance()->RegisterMsg();
	ServerConHandler::GetInstance()->SetServerId(m_conf.GetServerID());

	vector<ConnectionConfig> con = m_conf.GetConnCof();
	vector<ConnectionConfig>::iterator itCon = con.begin();
	for(; itCon!=con.end(); ++itCon)
	{
		ConnectionConfig connConf = *itCon;
		Message_Facade::AddConnectionConfig(connConf);
	}

	if(Message_Facade::Run())
	{
		LOG_ERROR(FILEINFO, " run net error ");

		return;
	}

	srand((unsigned int)time(0));
	DWORD64 bFps = 0;
	DWORD64 eFps = 0;

	sleep(1);
	while(!m_flag)
	{
		bFps = CUtil::GetNowSecond();

		try
		{
	#ifdef DEBUG
			int64 messagebegin = CUtil::GetNowSecond();
	#endif
			Message_Facade::ProcessMessage();
	#ifdef DEBUG
			messagetime = CUtil::GetNowSecond() - messagebegin;
	#endif
		}
		catch(exception &e)
		{
			LOG_ERROR(FILEINFO, "process message error [errmsg=%s]",e.what());
		}
		catch(...)
		{
			LOG_ERROR(FILEINFO, "process message unknown error");
		}

	#ifdef DEBUG
			int64 flushbegin = CUtil::GetNowSecond();
	#endif
		Message_Facade::MessageFlush();
	#ifdef DEBUG
		flushtime = CUtil::GetNowSecond() - flushbegin;
	#endif

		eFps = CUtil::GetNowSecond();
		eFps -= bFps;

		if(eFps < 100)
		{
			CUtil::MSleep(100 - eFps);
		}
		else
		{
			CUtil::MSleep(10);
		}
	}

	Message_Facade::Stop();

	UnInit();
}

void CServiceMain::Stop(int signal)
{
	m_flag = true;
//	Message_Facade::UnWait();
}


