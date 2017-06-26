/*
 * ServiceMain.cpp
 *
 *  Created on: Jun 11, 2017
 *      Author: root
 */
#include "ServiceMain.h"
#include <signal.h>
#include "./Log/Logger.h"
#include "./Network/Message_Facade.h"
#include "util.h"
#include "./Network/NetWorkConfig.h"
#include "ServerManager.h"
#include "CharactorMgr/CharactorLogin.h"

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

	strFullPath = strFileFullPath + "Config/GateServerConfig.lua";

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

void CServiceMain::InitInstance()
{
	CharactorLogin::GetInstance();
}

void CServiceMain::Start()
{
	if(Init())
	{
		cerr << " init config error " << endl;

		return ;
	}

	if(CLoggerMT::GetInstance()->Init(m_conf.GetLogConf().fileLen, m_conf.GetLogConf().info, m_conf.GetLogConf().debug,
			m_conf.GetLogConf().warning, m_conf.GetLogConf().error, m_conf.GetLogConf().fatal, m_conf.GetLogConf().display,
			m_conf.GetLogConf().filePath, m_conf.GetLogConf().module, m_conf.GetLogConf().base) != 0)
	{
		cerr << " init log error " << endl;

		return ;
	}

	if(InitFile() != 0)
	{
		return ;
	}

	if(Message_Facade::Init(ServerConHandler::GetInstance(), m_conf.GetBlockThread(), m_conf.GetIOThread()))
	{
		LOG_ERROR(FILEINFO, " init net error ");

		return;
	}

	ServerConHandler::GetInstance()->RegisterMsg();
	ServerConHandler::GetInstance()->SetServerID(m_conf.GetServerID());
	ServerConHandler::GetInstance()->SetIP(m_conf.GetFirstAccIP());
	ServerConHandler::GetInstance()->SetPort(m_conf.GetFirstAccPort());
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

	InitInstance();

	if(Message_Facade::Run())
	{
		LOG_ERROR(FILEINFO, " run net error ");

		return;
	}

	Message_Facade::Wait();
	Message_Facade::Stop();
	UnInit();
}

void CServiceMain::UnInit()
{
	CharactorLogin::GetInstance()->DestroyInstance();

	ServerConHandler::GetInstance()->DestoryInstance();
	CLoggerMT::GetInstance()->DestoryInstance();
}

void CServiceMain::Stop(int signal)
{
	Message_Facade::UnWait();
}
